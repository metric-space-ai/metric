// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_STRUCTURAL_ANALYSIS_DIAGNOSTICS_HPP
#define _METRIC_STATS_STRUCTURAL_ANALYSIS_DIAGNOSTICS_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/result.hpp>
#include <metric/record/id.hpp>
#include <metric/space/sample_plan.hpp>
#include <metric/space/storage/implicit.hpp>

namespace mtrc::stats::structural_analysis {

struct cluster_diagnostics_options {
	std::size_t max_exact_records{4096};
	std::size_t sample_count{512};
	bool allow_approximate{true};
};

// Cluster validity diagnostics that are well-defined for ANY finite metric space (they use
// only pairwise metric values, never coordinates). For each clustered (non-unassigned) record we
// compute the silhouette: a(i) = mean distance to the other members of its own cluster,
// b(i) = the smallest mean distance to any other cluster, s(i) = (b - a) / max(a, b) in
// [-1, 1] (Rousseeuw 1987). A singleton cluster, or a clustering with a single cluster,
// contributes s(i) = 0 by convention. Unassigned records are excluded from the evaluation.
//
// This is a read-only investigation of an existing clustering of an existing space; it
// neither mutates the space nor produces a new one.
template <typename Distance> struct ClusterDiagnostics {
	using distance_type = Distance;

	double mean_silhouette{};
	double mean_intra_cluster_distance{};
	double mean_nearest_cluster_distance{};
	std::size_t cluster_count{};
	std::size_t evaluated_record_count{};
	std::size_t unassigned_count{};
	std::vector<std::size_t> cluster_sizes;
	std::vector<double> per_cluster_mean_intra_distance;
	std::vector<RecordId> evaluated_ids;     // one entry per evaluated (non-unassigned) record
	std::vector<double> silhouettes;         // parallel to evaluated_ids
	std::size_t sample_count{};
	std::size_t sample_universe{};
	std::string approximation_reason;
	bool exact{true};
	std::string algorithm{"cluster_diagnostics"};
	std::string representation;

	auto empty() const -> bool { return evaluated_record_count == 0; }
};

template <typename Provider, typename Distance, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto cluster_diagnostics(const Provider &provider, const ClusteringResult<Distance> &clustering,
						 cluster_diagnostics_options options = {})
	-> ClusterDiagnostics<Distance>
{
	core::require_clustering_result_shape(clustering, provider.record_count(),
										  "cluster_diagnostics record_count does not match the provider",
										  "cluster_diagnostics assignments size does not match record_count");

	const auto unassigned_label = ClusteringResult<Distance>::unassigned_label;
	const auto cluster_count = clustering.cluster_count;

	ClusterDiagnostics<Distance> diagnostics;
	diagnostics.cluster_count = cluster_count;
	diagnostics.unassigned_count = clustering.unassigned_count;
	diagnostics.representation = clustering.representation;
	diagnostics.cluster_sizes.assign(cluster_count, 0);
	diagnostics.per_cluster_mean_intra_distance.assign(cluster_count, 0.0);

	// Bucket positions by cluster label (unassigned records excluded).
	std::vector<std::vector<std::size_t>> positions_by_cluster(cluster_count);
	for (std::size_t position = 0; position < clustering.assignments.size(); ++position) {
		const auto label = clustering.assignments[position];
		if (label == unassigned_label || label >= cluster_count) {
			continue;
		}
		positions_by_cluster[label].push_back(position);
		++diagnostics.cluster_sizes[label];
	}

	std::vector<std::size_t> evaluated_positions;
	for (const auto &members : positions_by_cluster) {
		evaluated_positions.insert(evaluated_positions.end(), members.begin(), members.end());
	}
	if (!options.allow_approximate && evaluated_positions.size() > options.max_exact_records) {
		throw RepresentationError(
			"cluster_diagnostics refused exact provider diagnostics before metric calls: records=" +
			std::to_string(provider.record_count()) +
			", evaluated_records=" + std::to_string(evaluated_positions.size()) +
			", max_exact_records=" + std::to_string(options.max_exact_records) +
			". Enable approximate diagnostics, lower sample_count, or pass a smaller clustering.");
	}

	bool sampled = false;
	if (options.allow_approximate && evaluated_positions.size() > options.max_exact_records) {
		if (options.sample_count == 0) {
			throw std::invalid_argument("cluster_diagnostics sample_count must be positive when sampling is enabled");
		}
		const auto sample_plan = ::mtrc::space::regular_sample_positions(evaluated_positions.size(), options.sample_count);
		std::vector<std::size_t> sampled_positions;
		sampled_positions.reserve(sample_plan.positions.size());
		for (const auto sample_position : sample_plan.positions) {
			sampled_positions.push_back(evaluated_positions[sample_position]);
		}
		evaluated_positions = std::move(sampled_positions);
		sampled = true;
		diagnostics.exact = false;
		diagnostics.algorithm = "sampled_cluster_diagnostics";
		diagnostics.sample_count = evaluated_positions.size();
		diagnostics.sample_universe = sample_plan.candidate_universe;
		diagnostics.approximation_reason =
			"cluster diagnostics evaluated on a deterministic regular sample of clustered records";
	} else {
		diagnostics.sample_count = evaluated_positions.size();
		diagnostics.sample_universe = evaluated_positions.size();
	}

	std::vector<double> per_cluster_intra_sum(cluster_count, 0.0);
	std::vector<std::size_t> per_cluster_intra_count(cluster_count, 0);

	double silhouette_sum = 0.0;
	double intra_sum = 0.0;
	double nearest_sum = 0.0;
	std::size_t nearest_count = 0;

	for (const auto position : evaluated_positions) {
		const auto label = clustering.assignments[position];
		if (label == unassigned_label || label >= cluster_count) {
			continue;
		}
		const auto cluster = label;
		const auto &members = positions_by_cluster[cluster];
		const auto source_id = provider.id(position);

		// a(i): mean distance to the other members of the same cluster.
		double own_sum = 0.0;
		for (const auto other : members) {
			if (other == position) {
				continue;
			}
			own_sum += static_cast<double>(provider.distance(source_id, provider.id(other)));
		}
		const double a_value = members.size() > 1 ? own_sum / static_cast<double>(members.size() - 1) : 0.0;

		// b(i): smallest mean distance to any other non-empty cluster.
		double b_value = 0.0;
		bool has_b = false;
		for (std::size_t other_cluster = 0; other_cluster < cluster_count; ++other_cluster) {
			if (other_cluster == cluster || positions_by_cluster[other_cluster].empty()) {
				continue;
			}
			double other_sum = 0.0;
			for (const auto other : positions_by_cluster[other_cluster]) {
				other_sum += static_cast<double>(provider.distance(source_id, provider.id(other)));
			}
			const double mean = other_sum / static_cast<double>(positions_by_cluster[other_cluster].size());
			if (!has_b || mean < b_value) {
				b_value = mean;
				has_b = true;
			}
		}

		double silhouette = 0.0;
		if (members.size() > 1 && has_b) {
			const double denominator = std::max(a_value, b_value);
			if (denominator > 0.0) {
				silhouette = (b_value - a_value) / denominator;
			}
		}

		diagnostics.evaluated_ids.push_back(source_id);
		diagnostics.silhouettes.push_back(silhouette);
		silhouette_sum += silhouette;
		intra_sum += a_value;
		per_cluster_intra_sum[cluster] += a_value;
		++per_cluster_intra_count[cluster];
		if (has_b) {
			nearest_sum += b_value;
			++nearest_count;
		}
	}
	if (sampled && diagnostics.sample_count == diagnostics.sample_universe) {
		diagnostics.exact = true;
		diagnostics.algorithm = "cluster_diagnostics";
		diagnostics.approximation_reason.clear();
	}

	diagnostics.evaluated_record_count = diagnostics.evaluated_ids.size();
	if (diagnostics.evaluated_record_count > 0) {
		diagnostics.mean_silhouette = silhouette_sum / static_cast<double>(diagnostics.evaluated_record_count);
		diagnostics.mean_intra_cluster_distance = intra_sum / static_cast<double>(diagnostics.evaluated_record_count);
	}
	if (nearest_count > 0) {
		diagnostics.mean_nearest_cluster_distance = nearest_sum / static_cast<double>(nearest_count);
	}
	for (std::size_t cluster = 0; cluster < cluster_count; ++cluster) {
		if (per_cluster_intra_count[cluster] > 0) {
			diagnostics.per_cluster_mean_intra_distance[cluster] =
				per_cluster_intra_sum[cluster] / static_cast<double>(per_cluster_intra_count[cluster]);
		}
	}

	return diagnostics;
}

template <typename Space, typename Distance, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto cluster_diagnostics(const Space &space, const ClusteringResult<Distance> &clustering,
						 cluster_diagnostics_options options = {})
	-> ClusterDiagnostics<Distance>
{
	space::storage::LiveDistances<Space> provider(space);
	auto diagnostics = cluster_diagnostics(provider, clustering, options);
	return diagnostics;
}

} // namespace mtrc::stats::structural_analysis

#endif
