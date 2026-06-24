// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Native metric.visual.v1 exporter for the Mixed Finite Metric Records example.
//
// The evidence below is computed in C++ from the existing mixed-record fixture
// and METRIC operators, then serialized through the shared visual writer.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <metric/engine.hpp>

#include "../../visual/cpp/mtrc_visual.hpp"
#include "mixed_finite_records.hpp"
#include "mixed_finite_records_fixture.hpp"

namespace {

namespace visual = mtrc::visual;

using hero::FlatEuclidean;
using hero::MixedRecord;
using hero::MixedRecordMetric;

constexpr const char *kDatasetId = "mixed-finite-records";
constexpr const char *kMetricRelationId = "mixed-finite-records-composite-metric";
constexpr const char *kSpaceId = "mixed-finite-records-space";
constexpr const char *kMdsCoordinateId = "mixed-finite-records-mds-3d";
constexpr const char *kSemanticCoordinateId = "mixed-finite-records-family-severity-3d";
constexpr double kEps = 1e-7;

constexpr MixedRecordMetric::Weights kWeights{
	/*code=*/0.6,
	/*spectrum=*/1.0,
	/*curve=*/0.5,
	/*vitals=*/1.0,
};

auto make_fleet_metric(const std::vector<MixedRecord> &records) -> MixedRecordMetric
{
	return MixedRecordMetric(hero::kSpectrumBins, hero::fit_vitals_metric(records), kWeights);
}

auto record_value(const std::string &record_id_value, const std::string &value_json) -> std::string
{
	return visual::object({visual::string_field("record_id", record_id_value), visual::field("value", value_json)});
}

auto record_id(std::size_t index) -> std::string
{
	std::ostringstream out;
	out << "fleet-" << std::setw(2) << std::setfill('0') << index;
	return out.str();
}

auto channel_names() -> std::vector<std::string> { return {"code", "spectrum", "curve", "vitals"}; }

auto contribution_vector(const hero::Contributions &contrib) -> std::vector<double>
{
	return {contrib.code, contrib.spectrum, contrib.curve, contrib.vitals};
}

auto dominant_channel(const hero::Contributions &contrib) -> std::string
{
	const auto values = contribution_vector(contrib);
	const auto names = channel_names();
	auto best = std::size_t{0};
	for (std::size_t i = 1; i < values.size(); ++i) {
		if (values[i] > values[best]) {
			best = i;
		}
	}
	return names[best];
}

auto clustering_purity(const std::vector<std::size_t> &assignments, const std::vector<std::string> &labels,
					   std::size_t cluster_count) -> double
{
	if (assignments.empty()) {
		return 0.0;
	}
	double correct = 0.0;
	for (std::size_t cluster = 0; cluster < cluster_count; ++cluster) {
		std::map<std::string, std::size_t> counts;
		for (std::size_t i = 0; i < assignments.size(); ++i) {
			if (assignments[i] == cluster) {
				++counts[labels[i]];
			}
		}
		std::size_t best = 0;
		for (const auto &entry : counts) {
			best = std::max(best, entry.second);
		}
		correct += static_cast<double>(best);
	}
	return correct / static_cast<double>(assignments.size());
}

auto record_payload(const MixedRecord &record, const std::string &family, double severity) -> std::string
{
	std::vector<double> spectrum_edges;
	spectrum_edges.reserve(hero::kSpectrumBins + 1);
	for (std::size_t i = 0; i <= hero::kSpectrumBins; ++i) {
		spectrum_edges.push_back(static_cast<double>(i));
	}

	return visual::object({
		visual::string_field("kind", "composed"),
		visual::field("fields",
					  visual::object({
						  visual::field("code", visual::string_payload(record.code)),
						  visual::field("spectrum", visual::object({visual::string_field("kind", "histogram"),
																	 visual::number_array_field("bins", record.spectrum),
																	 visual::number_array_field("edges", spectrum_edges)})),
						  visual::field("curve", visual::series_payload(record.curve, 1.0)),
						  visual::field("vitals", visual::object({visual::string_field("kind", "vector"),
																   visual::number_array_field("values", record.vitals),
																   visual::string_array_field(
																	   "names", {"severity_small", "family_large",
																				 "mixed_mid_a", "mixed_mid_b"})})),
					  })),
		visual::field("features", visual::object({visual::string_field("family", family),
												  visual::number_field("severity", severity),
												  visual::string_array_field("channels", channel_names())})),
	});
}

auto categorical_record_property(const std::string &id, const std::string &name,
								 const std::vector<std::string> &record_ids,
								 const std::vector<std::string> &values) -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(record_ids.size());
	for (std::size_t i = 0; i < record_ids.size(); ++i) {
		entries.push_back(record_value(record_ids[i], visual::quote(values[i])));
	}
	return visual::object({visual::string_field("id", id), visual::string_field("dataset_id", kDatasetId),
						   visual::string_field("target_type", "record"),
						   visual::string_field("value_type", "categorical"), visual::string_field("name", name),
						   visual::field("values", visual::array_of(entries))});
}

auto scalar_record_property(const std::string &id, const std::string &name, const std::vector<std::string> &record_ids,
							const std::vector<double> &values) -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(record_ids.size());
	for (std::size_t i = 0; i < record_ids.size(); ++i) {
		entries.push_back(record_value(record_ids[i], visual::num(values[i])));
	}
	return visual::object({visual::string_field("id", id), visual::string_field("dataset_id", kDatasetId),
						   visual::string_field("target_type", "record"),
						   visual::string_field("value_type", "scalar"), visual::string_field("name", name),
						   visual::field("values", visual::array_of(entries))});
}

auto boolean_record_property(const std::string &id, const std::string &name,
							 const std::vector<std::string> &record_ids, const std::vector<bool> &values) -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(record_ids.size());
	for (std::size_t i = 0; i < record_ids.size(); ++i) {
		entries.push_back(record_value(record_ids[i], visual::boolean(values[i])));
	}
	return visual::object({visual::string_field("id", id), visual::string_field("dataset_id", kDatasetId),
						   visual::string_field("target_type", "record"),
						   visual::string_field("value_type", "boolean"), visual::string_field("name", name),
						   visual::field("values", visual::array_of(entries))});
}

auto vector_record_property(const std::string &id, const std::string &name, const std::vector<std::string> &record_ids,
							const std::vector<std::vector<double>> &values) -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(record_ids.size());
	for (std::size_t i = 0; i < record_ids.size(); ++i) {
		entries.push_back(record_value(record_ids[i], visual::number_array(values[i])));
	}
	return visual::object({visual::string_field("id", id), visual::string_field("dataset_id", kDatasetId),
						   visual::string_field("target_type", "record"),
						   visual::string_field("value_type", "vector"), visual::string_field("name", name),
						   visual::field("values", visual::array_of(entries))});
}

struct LawDiagnostics {
	double min_distance{std::numeric_limits<double>::infinity()};
	double max_identity_error{};
	double max_symmetry_error{};
	double max_triangle_violation{};
	std::size_t triangle_violations{};
};

auto diagnose_metric_law(const std::vector<std::vector<double>> &distances) -> LawDiagnostics
{
	LawDiagnostics out;
	const auto n = distances.size();
	for (std::size_t i = 0; i < n; ++i) {
		out.max_identity_error = std::max(out.max_identity_error, std::abs(distances[i][i]));
		for (std::size_t j = 0; j < n; ++j) {
			out.min_distance = std::min(out.min_distance, distances[i][j]);
			out.max_symmetry_error = std::max(out.max_symmetry_error, std::abs(distances[i][j] - distances[j][i]));
			for (std::size_t k = 0; k < n; ++k) {
				const auto violation = distances[i][k] - distances[i][j] - distances[j][k];
				if (violation > out.max_triangle_violation) {
					out.max_triangle_violation = violation;
				}
				if (violation > kEps) {
					++out.triangle_violations;
				}
			}
		}
	}
	if (!std::isfinite(out.min_distance)) {
		out.min_distance = 0.0;
	}
	return out;
}

auto centered_metric_mds3(const std::vector<std::vector<double>> &distances) -> std::vector<std::vector<double>>
{
	const auto n = distances.size();
	if (n == 0) {
		return {};
	}

	std::vector<std::vector<double>> squared(n, std::vector<double>(n, 0.0));
	std::vector<double> row_mean(n, 0.0);
	double total_mean = 0.0;
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			squared[i][j] = distances[i][j] * distances[i][j];
			row_mean[i] += squared[i][j];
			total_mean += squared[i][j];
		}
		row_mean[i] /= static_cast<double>(n);
	}
	total_mean /= static_cast<double>(n * n);

	std::vector<std::vector<double>> matrix(n, std::vector<double>(n, 0.0));
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			matrix[i][j] = -0.5 * (squared[i][j] - row_mean[i] - row_mean[j] + total_mean);
		}
	}

	std::vector<std::vector<double>> vectors(n, std::vector<double>(n, 0.0));
	for (std::size_t i = 0; i < n; ++i) {
		vectors[i][i] = 1.0;
	}

	for (std::size_t iteration = 0; iteration < 80 * n * n; ++iteration) {
		std::size_t p = 0;
		std::size_t q = 1;
		double largest = 0.0;
		for (std::size_t i = 0; i < n; ++i) {
			for (std::size_t j = i + 1; j < n; ++j) {
				const auto value = std::abs(matrix[i][j]);
				if (value > largest) {
					largest = value;
					p = i;
					q = j;
				}
			}
		}
		if (largest < 1e-10 || n < 2) {
			break;
		}

		const auto app = matrix[p][p];
		const auto aqq = matrix[q][q];
		const auto apq = matrix[p][q];
		const auto angle = 0.5 * std::atan2(2.0 * apq, aqq - app);
		const auto c = std::cos(angle);
		const auto s = std::sin(angle);

		for (std::size_t k = 0; k < n; ++k) {
			if (k == p || k == q) {
				continue;
			}
			const auto mkp = matrix[k][p];
			const auto mkq = matrix[k][q];
			matrix[k][p] = matrix[p][k] = c * mkp - s * mkq;
			matrix[k][q] = matrix[q][k] = s * mkp + c * mkq;
		}

		matrix[p][p] = c * c * app - 2.0 * s * c * apq + s * s * aqq;
		matrix[q][q] = s * s * app + 2.0 * s * c * apq + c * c * aqq;
		matrix[p][q] = matrix[q][p] = 0.0;

		for (std::size_t k = 0; k < n; ++k) {
			const auto vip = vectors[k][p];
			const auto viq = vectors[k][q];
			vectors[k][p] = c * vip - s * viq;
			vectors[k][q] = s * vip + c * viq;
		}
	}

	std::vector<std::size_t> order(n);
	std::iota(order.begin(), order.end(), 0);
	std::sort(order.begin(), order.end(), [&](std::size_t lhs, std::size_t rhs) {
		return matrix[lhs][lhs] > matrix[rhs][rhs];
	});

	std::vector<std::vector<double>> positions(n, std::vector<double>(3, 0.0));
	for (std::size_t dimension = 0; dimension < std::min<std::size_t>(3, n); ++dimension) {
		const auto eigen_index = order[dimension];
		const auto lambda = matrix[eigen_index][eigen_index];
		if (lambda <= 0.0) {
			continue;
		}
		const auto scale = std::sqrt(lambda);
		for (std::size_t record = 0; record < n; ++record) {
			positions[record][dimension] = vectors[record][eigen_index] * scale;
		}
	}

	double max_abs = 0.0;
	for (const auto &position : positions) {
		for (const auto value : position) {
			max_abs = std::max(max_abs, std::abs(value));
		}
	}
	if (max_abs > 0.0) {
		for (auto &position : positions) {
			for (auto &value : position) {
				value = 1.35 * value / max_abs;
			}
		}
	}
	return positions;
}

auto semantic_positions(const std::vector<hero::LabeledRecord> &fleet) -> std::vector<std::vector<double>>
{
	std::vector<std::vector<double>> positions;
	positions.reserve(fleet.size());
	for (std::size_t i = 0; i < fleet.size(); ++i) {
		const auto family = i / hero::kSeverityLevels;
		const auto severity = static_cast<double>(i % hero::kSeverityLevels);
		const auto angle = (static_cast<double>(family) / static_cast<double>(hero::kFamilyCount)) * 2.0 * std::acos(-1.0);
		const auto radius = 0.75 + 0.11 * severity;
		positions.push_back({radius * std::cos(angle), 0.28 * severity, radius * std::sin(angle)});
	}
	return positions;
}

auto coordinates_json(const std::string &id, const std::string &name, const std::vector<std::string> &record_ids,
					  const std::vector<std::vector<double>> &positions) -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(record_ids.size());
	for (std::size_t i = 0; i < record_ids.size(); ++i) {
		entries.push_back(visual::object({visual::string_field("record_id", record_ids[i]),
										   visual::number_array_field("position", positions[i])}));
	}
	return visual::object({visual::string_field("id", id), visual::string_field("dataset_id", kDatasetId),
						   visual::string_field("space_id", kSpaceId), visual::string_field("name", name),
						   visual::size_field("dimension", 3), visual::field("record_positions", visual::array_of(entries))});
}

auto nearest_indices(const std::vector<std::vector<double>> &distances) -> std::vector<std::size_t>
{
	std::vector<std::size_t> nearest(distances.size(), 0);
	for (std::size_t i = 0; i < distances.size(); ++i) {
		double best = std::numeric_limits<double>::infinity();
		for (std::size_t j = 0; j < distances.size(); ++j) {
			if (i == j) {
				continue;
			}
			if (distances[i][j] < best) {
				best = distances[i][j];
				nearest[i] = j;
			}
		}
	}
	return nearest;
}

auto top_k_edges(const std::vector<std::string> &record_ids, const std::vector<std::vector<double>> &distances,
				 std::size_t k) -> std::vector<std::string>
{
	std::vector<std::string> edges;
	for (std::size_t i = 0; i < distances.size(); ++i) {
		std::vector<std::pair<double, std::size_t>> neighbors;
		neighbors.reserve(distances.size() - 1);
		for (std::size_t j = 0; j < distances.size(); ++j) {
			if (i != j) {
				neighbors.push_back({distances[i][j], j});
			}
		}
		std::sort(neighbors.begin(), neighbors.end());
		for (std::size_t n = 0; n < std::min(k, neighbors.size()); ++n) {
			edges.push_back(visual::object({visual::string_field("source", record_ids[i]),
											visual::string_field("target", record_ids[neighbors[n].second]),
											visual::number_field("value", neighbors[n].first)}));
		}
	}
	return edges;
}

struct ProbeDiagnostic {
	std::string channel;
	std::string target_label;
	std::string metric_label;
	std::string flat_label;
	double target_distance{};
	double decoy_distance{};
	bool metric_ok{};
	bool flat_wrong{};
};

auto probe_diagnostic_json(const std::vector<ProbeDiagnostic> &probes) -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(probes.size());
	for (const auto &probe : probes) {
		entries.push_back(visual::object({
			visual::string_field("channel", probe.channel),
			visual::string_field("target_label", probe.target_label),
			visual::string_field("metric_label", probe.metric_label),
			visual::string_field("flat_label", probe.flat_label),
			visual::number_field("metric_distance_to_target", probe.target_distance),
			visual::number_field("metric_distance_to_decoy", probe.decoy_distance),
			visual::bool_field("metric_ok", probe.metric_ok),
			visual::bool_field("flat_wrong", probe.flat_wrong),
		}));
	}
	return visual::array_of(entries);
}

auto build_visual_document() -> std::string
{
	const auto fleet = hero::make_fleet();
	const auto records = hero::fleet_records(fleet);
	const auto metric = make_fleet_metric(records);
	auto space = mtrc::make_space(records, metric);

	const auto cover_tree = mtrc::space::storage::cover_tree(space);
	const auto record_count = records.size();

	std::vector<std::string> record_ids;
	std::vector<std::string> family_values;
	std::vector<double> severity_values;
	record_ids.reserve(record_count);
	family_values.reserve(record_count);
	severity_values.reserve(record_count);
	for (std::size_t i = 0; i < record_count; ++i) {
		record_ids.push_back(record_id(i));
		family_values.push_back(fleet[i].family);
		severity_values.push_back(fleet[i].severity);
	}

	std::vector<std::vector<double>> distances(record_count, std::vector<double>(record_count, 0.0));
	for (std::size_t i = 0; i < record_count; ++i) {
		for (std::size_t j = 0; j < record_count; ++j) {
			distances[i][j] = metric(records[i], records[j]);
		}
	}
	const auto law = diagnose_metric_law(distances);
	const bool metric_law_ok = law.min_distance >= -kEps && law.max_identity_error <= kEps &&
							   law.max_symmetry_error <= kEps && law.triangle_violations == 0;

	const auto nearest = nearest_indices(distances);
	std::vector<std::string> dominant_channels;
	std::vector<double> nearest_distances;
	std::vector<std::vector<double>> nearest_contributions;
	dominant_channels.reserve(record_count);
	nearest_distances.reserve(record_count);
	nearest_contributions.reserve(record_count);
	for (std::size_t i = 0; i < record_count; ++i) {
		const auto contributions = metric.contributions(records[i], records[nearest[i]]);
		dominant_channels.push_back(dominant_channel(contributions));
		nearest_distances.push_back(distances[i][nearest[i]]);
		nearest_contributions.push_back(contribution_vector(contributions));
	}

	const auto query_index = std::size_t{2};
	const auto brute = mtrc::find_neighbors(space, records[query_index], mtrc::count{3});
	const auto tree =
		mtrc::find_neighbors(space, records[query_index], mtrc::count{3}, mtrc::stats::search::cover_tree{});
	const bool search_parity_ok = brute[0].id == tree[0].id;

	const auto catalog = hero::make_demo_catalog();
	const auto catalog_metric = make_fleet_metric(catalog.records);
	auto catalog_space = mtrc::make_space(catalog.records, catalog_metric);
	auto flat_catalog_space = mtrc::make_space(hero::flat_projection(catalog.records), FlatEuclidean{});

	std::size_t probe_metric_correct = 0;
	std::size_t probe_flat_wrong = 0;
	std::vector<ProbeDiagnostic> probe_details;
	for (const auto &probe : catalog.probes) {
		const auto metric_hit = mtrc::find_neighbors(catalog_space, probe.query, mtrc::count{1});
		const auto flat_hit =
			mtrc::find_neighbors(flat_catalog_space, hero::flat_projection(probe.query), mtrc::count{1});
		const auto metric_label = catalog.labels[metric_hit[0].id.index()];
		const auto flat_label = catalog.labels[flat_hit[0].id.index()];

		const auto target_idx = hero::label_index(catalog.labels, probe.target_label);
		const auto decoy_idx = hero::label_index(catalog.labels, probe.decoy_label);
		const auto d_target = catalog_metric.contributions(probe.query, catalog.records[target_idx]);
		const auto d_decoy = catalog_metric.contributions(probe.query, catalog.records[decoy_idx]);

		const bool metric_ok = metric_label == probe.target_label;
		const bool flat_off = flat_label != probe.target_label;
		probe_metric_correct += metric_ok ? 1 : 0;
		probe_flat_wrong += flat_off ? 1 : 0;
		probe_details.push_back({probe.channel, probe.target_label, metric_label, flat_label, d_target.total(),
								 d_decoy.total(), metric_ok, flat_off});
	}
	const bool probes_ok = probe_metric_correct == catalog.probes.size() && probe_flat_wrong == catalog.probes.size();

	const auto reps = mtrc::find_representatives(space, hero::kFamilyCount);
	std::vector<bool> representative_values(record_count, false);
	std::vector<std::string> representative_ids;
	for (std::size_t i = 0; i < reps.size(); ++i) {
		representative_values[reps[i].index()] = true;
		representative_ids.push_back(record_ids[reps[i].index()]);
	}

	const auto policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	const auto fleet_scaler = hero::fit_flat_standardizer(hero::flat_projection(records));
	auto flat_space = mtrc::make_space(hero::standardized_flat_projection(records, fleet_scaler), FlatEuclidean{});
	auto raw_flat_space = mtrc::make_space(hero::flat_projection(records), FlatEuclidean{});

	const auto family_groups =
		mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(hero::kFamilyCount), policy);
	const auto flat_family_groups =
		mtrc::find_groups(flat_space, mtrc::stats::structural_analysis::k_medoids_options(hero::kFamilyCount), policy);
	const auto metric_family_purity =
		clustering_purity(family_groups.assignments, family_values, family_groups.cluster_count);
	const auto flat_family_purity =
		clustering_purity(flat_family_groups.assignments, family_values, flat_family_groups.cluster_count);
	std::vector<double> cluster_values;
	cluster_values.reserve(family_groups.assignments.size());
	for (const auto assignment : family_groups.assignments) {
		cluster_values.push_back(static_cast<double>(assignment));
	}

	auto records_with_anomaly = records;
	records_with_anomaly.push_back(hero::make_anomaly());
	const auto anomaly_metric = make_fleet_metric(records_with_anomaly);
	auto anomaly_space = mtrc::make_space(records_with_anomaly, anomaly_metric);
	const auto anomaly_index = records_with_anomaly.size() - 1;
	const auto outliers =
		mtrc::find_outliers(anomaly_space, mtrc::stats::structural_analysis::dbscan_options(9.0, 2), policy);
	bool anomaly_flagged = false;
	std::vector<std::string> outlier_indices;
	for (const auto &outlier : outliers) {
		if (outlier.id.index() == anomaly_index) {
			anomaly_flagged = true;
		}
		outlier_indices.push_back(std::to_string(outlier.id.index()));
	}

	const auto outcome_records = hero::fleet_outcomes(fleet);
	auto outcome_space = mtrc::make_space(outcome_records, mtrc::Euclidean<double>{});
	const auto mgc_metric = mtrc::compare(space, outcome_space, mtrc::stats::correlate::mgc_options{});
	const auto mgc_flat = mtrc::compare(flat_space, outcome_space, mtrc::stats::correlate::mgc_options{});
	const auto mgc_raw_flat = mtrc::compare(raw_flat_space, outcome_space, mtrc::stats::correlate::mgc_options{});
	const bool mgc_ok = mgc_metric.value > mgc_flat.value;

	const bool invariant_ok = metric_law_ok && search_parity_ok && probes_ok && reps.size() == hero::kFamilyCount &&
							  metric_family_purity == 1.0 && metric_family_purity >= flat_family_purity &&
							  anomaly_flagged && mgc_ok;

	std::vector<std::string> dataset_entries;
	dataset_entries.push_back(visual::object({
		visual::string_field("id", kDatasetId),
		visual::string_field("title", "Mixed Finite Metric Records"),
		visual::string_field("description",
							 "Native C++ export of the mixed finite records fixture using a composite admitted METRIC metric."),
		visual::string_field("source", "examples/engine/mixed_finite_records_fixture.hpp"),
		visual::string_field("license", "MPL-2.0"),
	}));

	std::vector<std::string> record_entries;
	record_entries.reserve(record_count);
	for (std::size_t i = 0; i < record_count; ++i) {
		const auto label = fleet[i].family + " severity " + std::to_string(static_cast<int>(fleet[i].severity));
		record_entries.push_back(visual::object({
			visual::string_field("id", record_ids[i]),
			visual::string_field("dataset_id", kDatasetId),
			visual::string_field("record_type", "mixed_structured_record"),
			visual::string_field("label", label),
			visual::field("payload", record_payload(records[i], fleet[i].family, fleet[i].severity)),
		}));
	}

	std::vector<std::string> metric_values;
	metric_values.reserve(record_count * record_count);
	for (std::size_t i = 0; i < record_count; ++i) {
		for (std::size_t j = 0; j < record_count; ++j) {
			metric_values.push_back(visual::object({visual::string_field("row_id", record_ids[i]),
													visual::string_field("column_id", record_ids[j]),
													visual::number_field("value", distances[i][j])}));
		}
	}

	const auto relation_metadata = visual::object({
		visual::bool_field("symmetric", true),
		visual::bool_field("complete", true),
		visual::string_field("metric_law", mtrc::metric_law_name(mtrc::metric_traits<MixedRecordMetric>::law)),
		visual::string_field("metric_key", mtrc::metric_cache_key(metric)),
		visual::field("weights", visual::object({visual::number_field("code", kWeights.code),
												  visual::number_field("spectrum", kWeights.spectrum),
												  visual::number_field("curve", kWeights.curve),
												  visual::number_field("vitals", kWeights.vitals)})),
		visual::field("component_metrics",
					  visual::object({visual::string_field("code", "mtrc::Edit<char>"),
									  visual::string_field("spectrum", "mtrc::Wasserstein<double>::on_line"),
									  visual::string_field("curve", "mtrc::TWED<double>"),
									  visual::string_field("vitals", "mtrc::Euclidean_standardized<double>")})),
		visual::field("law_check", visual::object({visual::string_field("checked", "full finite fleet matrix"),
												   visual::bool_field("non_negative", law.min_distance >= -kEps),
												   visual::bool_field("identity", law.max_identity_error <= kEps),
												   visual::bool_field("symmetry", law.max_symmetry_error <= kEps),
												   visual::bool_field("triangle", law.triangle_violations == 0),
												   visual::number_field("min_distance", law.min_distance),
												   visual::number_field("max_identity_error", law.max_identity_error),
												   visual::number_field("max_symmetry_error", law.max_symmetry_error),
												   visual::number_field("max_triangle_violation",
																		law.max_triangle_violation),
												   visual::size_field("triangle_violations", law.triangle_violations)})),
	});

	std::vector<std::string> relation_entries;
	relation_entries.push_back(visual::object({
		visual::string_field("id", kMetricRelationId),
		visual::string_field("dataset_id", kDatasetId),
		visual::string_field("name", "composite mixed-record metric"),
		visual::string_field("relation_type", "metric"),
		visual::string_field("value_type", "scalar"),
		visual::string_array_field("record_ids", record_ids),
		visual::string_field("storage", "sparse_edge_list"),
		visual::field("values", visual::array_of(metric_values)),
		visual::field("metadata", relation_metadata),
	}));

	std::vector<std::string> space_entries;
	space_entries.push_back(visual::object({
		visual::string_field("id", kSpaceId),
		visual::string_field("dataset_id", kDatasetId),
		visual::string_array_field("record_ids", record_ids),
		visual::string_field("primary_relation_id", kMetricRelationId),
		visual::string_field("space_type", "finite_metric_space"),
		visual::field("metadata", visual::object({visual::size_field("record_count", record_count),
												   visual::string_field("metric_law", "metric"),
												   visual::bool_field("native_export_foundation", true),
												   visual::bool_field("public_hero_ready", false)})),
	}));

	std::vector<std::string> type_values(record_count, "mixed_structured_record");
	std::vector<std::string> property_entries;
	property_entries.push_back(categorical_record_property("family", "fault family", record_ids, family_values));
	property_entries.push_back(scalar_record_property("severity", "severity level", record_ids, severity_values));
	property_entries.push_back(categorical_record_property("type", "record type", record_ids, type_values));
	property_entries.push_back(categorical_record_property("channel", "dominant nearest-neighbor channel", record_ids,
														   dominant_channels));
	property_entries.push_back(scalar_record_property("nearest_distance", "nearest metric distance", record_ids,
													  nearest_distances));
	property_entries.push_back(vector_record_property("nearest_contributions",
													  "weighted channel contributions to nearest record", record_ids,
													  nearest_contributions));
	property_entries.push_back(scalar_record_property("cluster", "k-medoids cluster", record_ids, cluster_values));
	property_entries.push_back(boolean_record_property("representative", "farthest-first representative", record_ids,
													   representative_values));

	const auto knn_edges = top_k_edges(record_ids, distances, 3);
	std::vector<std::string> graph_entries;
	graph_entries.push_back(visual::object({
		visual::string_field("id", "mixed-finite-records-knn"),
		visual::string_field("dataset_id", kDatasetId),
		visual::string_array_field("node_record_ids", record_ids),
		visual::string_field("edge_relation_id", kMetricRelationId),
		visual::string_field("graph_type", "k-nearest"),
		visual::field("edges", visual::array_of(knn_edges)),
		visual::field("metadata", visual::object({visual::size_field("neighbors", 3),
												   visual::string_field("edge_source", "native composite metric")})),
	}));

	std::vector<std::string> coordinate_entries;
	coordinate_entries.push_back(coordinates_json(kMdsCoordinateId, "classical MDS from composite metric",
												  record_ids, centered_metric_mds3(distances)));
	coordinate_entries.push_back(coordinates_json(kSemanticCoordinateId, "fixture family/severity layout",
												  record_ids, semantic_positions(fleet)));

	std::vector<std::string> view_entries;
	view_entries.push_back(visual::object({visual::string_field("id", "mixed-finite-records-space-view"),
											visual::string_field("kind", "metric-space"),
											visual::string_field("name", "Mixed finite records metric space"),
											visual::string_field("spaceId", kSpaceId),
											visual::string_field("coordinateId", kMdsCoordinateId),
											visual::string_field("propertyId", "family")}));
	view_entries.push_back(visual::object({visual::string_field("id", "mixed-finite-records-relation-view"),
											visual::string_field("kind", "relation-matrix"),
											visual::string_field("name", "Composite metric matrix"),
											visual::string_field("relationId", kMetricRelationId)}));

	std::vector<std::string> diagnostic_entries;
	diagnostic_entries.push_back(visual::object({
		visual::string_field("id", "export-status"),
		visual::string_field("kind", "native_export_foundation"),
		visual::bool_field("ok", true),
		visual::string_field("generator", "examples/engine/mixed_finite_records_visual_export.cpp"),
		visual::bool_field("synthetic_js", false),
		visual::bool_field("public_hero_ready", false),
		visual::string_field("note",
							 "Native C++ metric.visual.v1 export foundation; not a public hero readiness claim."),
	}));
	diagnostic_entries.push_back(visual::object({visual::string_field("id", "metric-law-check"),
												 visual::string_field("kind", "law_check"),
												 visual::bool_field("ok", metric_law_ok),
												 visual::string_field("relation_id", kMetricRelationId),
												 visual::field("checks", relation_metadata)}));
	diagnostic_entries.push_back(visual::object({visual::string_field("id", "cover-tree-admission"),
												 visual::string_field("kind", "metric_admission"),
												 visual::bool_field("ok", true),
												 visual::string_field("representation", "cover_tree_index"),
												 visual::size_field("records", cover_tree.record_count())}));
	diagnostic_entries.push_back(visual::object({
		visual::string_field("id", "search-parity"),
		visual::string_field("kind", "nearest_neighbor_check"),
		visual::bool_field("ok", search_parity_ok),
		visual::string_field("query_record_id", record_ids[query_index]),
		visual::string_field("brute_representation", brute.representation),
		visual::string_field("cover_tree_representation", tree.representation),
		visual::string_field("brute_nearest", record_ids[brute[0].id.index()]),
		visual::string_field("cover_tree_nearest", record_ids[tree[0].id.index()]),
	}));
	diagnostic_entries.push_back(visual::object({visual::string_field("id", "channel-probes"),
												 visual::string_field("kind", "baseline_probe_summary"),
												 visual::bool_field("ok", probes_ok),
												 visual::size_field("probe_count", catalog.probes.size()),
												 visual::size_field("composite_correct", probe_metric_correct),
												 visual::size_field("flat_wrong", probe_flat_wrong),
												 visual::field("probes", probe_diagnostic_json(probe_details))}));
	diagnostic_entries.push_back(visual::object({
		visual::string_field("id", "representatives"),
		visual::string_field("kind", "farthest_first_representatives"),
		visual::bool_field("ok", reps.size() == hero::kFamilyCount),
		visual::string_field("strategy", reps.strategy),
		visual::number_field("coverage_radius", reps.coverage_radius),
		visual::number_field("average_nearest_distance", reps.average_nearest_distance),
		visual::string_array_field("record_ids", representative_ids),
	}));
	diagnostic_entries.push_back(visual::object({
		visual::string_field("id", "family-clustering"),
		visual::string_field("kind", "k_medoids_family_purity"),
		visual::bool_field("ok", metric_family_purity == 1.0 && metric_family_purity >= flat_family_purity),
		visual::string_field("algorithm", family_groups.algorithm),
		visual::size_field("cluster_count", family_groups.cluster_count),
		visual::number_field("composite_family_purity", metric_family_purity),
		visual::number_field("standardized_flat_family_purity", flat_family_purity),
	}));
	diagnostic_entries.push_back(visual::object({visual::string_field("id", "anomaly-outlier"),
												 visual::string_field("kind", "dbscan_outlier_check"),
												 visual::bool_field("ok", anomaly_flagged),
												 visual::integer_field("radius", 9),
												 visual::integer_field("min_points", 2),
												 visual::size_field("flagged_count", outliers.size()),
												 visual::size_field("injected_anomaly_index", anomaly_index),
												 visual::string_array_field("flagged_indices", outlier_indices)}));
	diagnostic_entries.push_back(visual::object({
		visual::string_field("id", "severity-mgc"),
		visual::string_field("kind", "cross_space_mgc"),
		visual::bool_field("ok", mgc_ok),
		visual::string_field("algorithm", mgc_metric.algorithm),
		visual::number_field("composite", mgc_metric.value),
		visual::number_field("standardized_flat", mgc_flat.value),
		visual::number_field("raw_flat", mgc_raw_flat.value),
		visual::number_field("composite_advantage", mgc_metric.value - mgc_flat.value),
	}));
	diagnostic_entries.push_back(visual::object({
		visual::string_field("id", "existing-check-summary"),
		visual::string_field("kind", "mixed_finite_records_checks"),
		visual::bool_field("ok", invariant_ok),
		visual::field("checks", visual::object({visual::bool_field("metric_law", true),
												 visual::bool_field("search_parity", search_parity_ok),
												 visual::bool_field("channel_probes", probes_ok),
												 visual::bool_field("representatives", reps.size() == hero::kFamilyCount),
												 visual::bool_field("family_purity", metric_family_purity == 1.0),
												 visual::bool_field("anomaly_flagged", anomaly_flagged),
												 visual::bool_field("mgc_advantage", mgc_ok)})),
	}));

	visual::Document doc;
	doc.provenance_json(visual::object({
		visual::string_field("generator", "examples/engine/mixed_finite_records_visual_export.cpp"),
		visual::string_field("runtime", "native C++"),
		visual::string_field("source", "examples/engine/mixed_finite_records_fixture.hpp"),
		visual::bool_field("native_export", true),
		visual::bool_field("synthetic_js", false),
		visual::bool_field("public_hero_ready", false),
		visual::string_field("status", "native_export_foundation"),
	}));
	for (const auto &entry : dataset_entries) {
		doc.dataset_json(entry);
	}
	for (const auto &entry : record_entries) {
		doc.record_json(entry);
	}
	for (const auto &entry : relation_entries) {
		doc.relation_json(entry);
	}
	for (const auto &entry : space_entries) {
		doc.space_json(entry);
	}
	for (const auto &entry : property_entries) {
		doc.property_json(entry);
	}
	for (const auto &entry : graph_entries) {
		doc.graph_json(entry);
	}
	for (const auto &entry : coordinate_entries) {
		doc.coordinates_json(entry);
	}
	for (const auto &entry : view_entries) {
		doc.view_json(entry);
	}
	for (const auto &entry : diagnostic_entries) {
		doc.diagnostic_json(entry);
	}
	return doc.to_json() + "\n";
}

auto usage(const char *program) -> std::string
{
	std::ostringstream out;
	out << "usage: " << program << " [--export-dir <dir>]\n";
	out << "  without --export-dir, writes metric.visual.v1 JSON to stdout\n";
	return out.str();
}

} // namespace

int main(int argc, char **argv)
{
	try {
		std::filesystem::path export_dir;
		for (int i = 1; i < argc; ++i) {
			const std::string arg = argv[i];
			if (arg == "--help" || arg == "-h") {
				std::cout << usage(argv[0]);
				return 0;
			}
			if (arg == "--export-dir") {
				if (i + 1 >= argc) {
					throw std::invalid_argument("--export-dir requires a directory argument");
				}
				export_dir = argv[++i];
				continue;
			}
			throw std::invalid_argument("unknown argument: " + arg);
		}

		const auto json = build_visual_document();
		if (export_dir.empty()) {
			std::cout << json;
			return 0;
		}

		const auto output_path = export_dir / "metric.visual.json";
		if (!visual::write_metric_visual_file(export_dir, json)) {
			throw std::runtime_error("failed to write output file: " + output_path.string());
		}
		std::cerr << "wrote " << output_path << "\n";
		return 0;
	} catch (const std::exception &err) {
		std::cerr << "mixed_finite_records_visual_export: " << err.what() << "\n";
		std::cerr << usage(argv[0]);
		return 1;
	}
}
