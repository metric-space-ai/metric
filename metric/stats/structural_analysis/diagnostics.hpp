// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_STRUCTURAL_ANALYSIS_DIAGNOSTICS_HPP
#define _METRIC_STATS_STRUCTURAL_ANALYSIS_DIAGNOSTICS_HPP

#include <algorithm>
#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/result.hpp>
#include <metric/record/id.hpp>
#include <metric/space/storage/implicit.hpp>

namespace mtrc::stats::structural_analysis {

// Cluster validity diagnostics that are well-defined for ANY finite metric space (they use
// only pairwise metric values, never coordinates). For each clustered (non-noise) record we
// compute the silhouette: a(i) = mean distance to the other members of its own cluster,
// b(i) = the smallest mean distance to any other cluster, s(i) = (b - a) / max(a, b) in
// [-1, 1] (Rousseeuw 1987). A singleton cluster, or a clustering with a single cluster,
// contributes s(i) = 0 by convention. Noise records are excluded from the evaluation.
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
	std::size_t noise_count{};
	std::vector<std::size_t> cluster_sizes;
	std::vector<double> per_cluster_mean_intra_distance;
	std::vector<RecordId> evaluated_ids;     // one entry per evaluated (non-noise) record
	std::vector<double> silhouettes;         // parallel to evaluated_ids
	bool exact{true};
	std::string algorithm{"cluster_diagnostics"};
	std::string representation;

	auto empty() const -> bool { return evaluated_record_count == 0; }
};

template <typename Provider, typename Distance, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto cluster_diagnostics(const Provider &provider, const ClusteringResult<Distance> &clustering)
	-> ClusterDiagnostics<Distance>
{
	core::require_clustering_result_shape(clustering, provider.record_count(),
										  "cluster_diagnostics record_count does not match the provider",
										  "cluster_diagnostics assignments size does not match record_count");

	const auto noise_label = ClusteringResult<Distance>::noise_label;
	const auto cluster_count = clustering.cluster_count;

	ClusterDiagnostics<Distance> diagnostics;
	diagnostics.cluster_count = cluster_count;
	diagnostics.noise_count = clustering.noise_count;
	diagnostics.representation = clustering.representation;
	diagnostics.cluster_sizes.assign(cluster_count, 0);
	diagnostics.per_cluster_mean_intra_distance.assign(cluster_count, 0.0);

	// Bucket positions by cluster label (noise excluded).
	std::vector<std::vector<std::size_t>> positions_by_cluster(cluster_count);
	for (std::size_t position = 0; position < clustering.assignments.size(); ++position) {
		const auto label = clustering.assignments[position];
		if (label == noise_label || label >= cluster_count) {
			continue;
		}
		positions_by_cluster[label].push_back(position);
		++diagnostics.cluster_sizes[label];
	}

	std::vector<double> per_cluster_intra_sum(cluster_count, 0.0);
	std::vector<std::size_t> per_cluster_intra_count(cluster_count, 0);

	double silhouette_sum = 0.0;
	double intra_sum = 0.0;
	double nearest_sum = 0.0;
	std::size_t nearest_count = 0;

	for (std::size_t cluster = 0; cluster < cluster_count; ++cluster) {
		const auto &members = positions_by_cluster[cluster];
		for (const auto position : members) {
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
auto cluster_diagnostics(const Space &space, const ClusteringResult<Distance> &clustering)
	-> ClusterDiagnostics<Distance>
{
	space::storage::LiveDistances<Space> provider(space);
	auto diagnostics = cluster_diagnostics(provider, clustering);
	return diagnostics;
}

} // namespace mtrc::stats::structural_analysis

#endif
