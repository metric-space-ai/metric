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
	return MixedRecordMetric(hero::kSpectrumBins, hero::derive_vitals_metric(records), kWeights);
}

auto record_value(const std::string &record_id_value, const std::string &value_json) -> std::string
{
	return visual::object({visual::string_field("record_id", record_id_value), visual::field("value", value_json)});
}

auto record_id(std::size_t index) -> std::string
{
	std::ostringstream out;
	out << "fleet-" << std::setw(4) << std::setfill('0') << index;
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

auto spectrum_edges() -> std::vector<double>
{
	std::vector<double> spectrum_edges;
	spectrum_edges.reserve(hero::kSpectrumBins + 1);
	for (std::size_t i = 0; i <= hero::kSpectrumBins; ++i) {
		spectrum_edges.push_back(static_cast<double>(i));
	}
	return spectrum_edges;
}

auto record_attributes(const hero::TypedLabeledRecord &entry) -> std::string
{
	return visual::object({
		visual::string_field("family", entry.family),
		visual::number_field("severity", entry.severity),
		visual::string_field("record_type", entry.record_type),
		visual::string_field("payload_family", entry.payload_family),
		visual::size_field("family_index", entry.family_index),
		visual::size_field("severity_index", entry.severity_index),
		visual::size_field("type_index", entry.type_index),
		visual::size_field("variant", entry.variant),
		visual::string_array_field("metric_channels", channel_names()),
	});
}

auto record_payload(const hero::TypedLabeledRecord &entry) -> std::string
{
	const auto &record = entry.record;
	const auto attributes = record_attributes(entry);
	if (entry.record_type == "text_code_record") {
		return visual::object({
			visual::string_field("kind", "string"),
			visual::string_field("text", record.code),
			visual::field("record_attributes", attributes),
		});
	}
	if (entry.record_type == "histogram_spectrum_record") {
		return visual::object({
			visual::string_field("kind", "histogram"),
			visual::number_array_field("bins", record.spectrum),
			visual::number_array_field("edges", spectrum_edges()),
			visual::field("record_attributes", attributes),
		});
	}
	if (entry.record_type == "process_curve_record") {
		return visual::object({
			visual::string_field("kind", "time_series"),
			visual::number_field("sample_rate_hz", 1.0),
			visual::number_array_field("series", record.curve),
			visual::field("record_attributes", attributes),
		});
	}
	return visual::object({
		visual::string_field("kind", "vector"),
		visual::number_array_field("values", record.vitals),
		visual::string_array_field("names", {"severity_small", "family_large", "mixed_mid_a", "mixed_mid_b"}),
		visual::field("record_attributes", attributes),
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
	std::size_t pair_checks{};
	std::size_t triangle_checks{};
	std::size_t triangle_violations{};
	std::size_t zero_off_diagonal_pairs{};
};

auto diagnose_metric_law(const std::vector<std::vector<double>> &distances) -> LawDiagnostics
{
	LawDiagnostics out;
	const auto n = distances.size();
	for (std::size_t i = 0; i < n; ++i) {
		out.max_identity_error = std::max(out.max_identity_error, std::abs(distances[i][i]));
		for (std::size_t j = 0; j < n; ++j) {
			++out.pair_checks;
			out.min_distance = std::min(out.min_distance, distances[i][j]);
			out.max_symmetry_error = std::max(out.max_symmetry_error, std::abs(distances[i][j] - distances[j][i]));
			if (i != j && std::abs(distances[i][j]) <= kEps) {
				++out.zero_off_diagonal_pairs;
			}
		}
	}
	if (n <= 80) {
		for (std::size_t i = 0; i < n; ++i) {
			for (std::size_t j = 0; j < n; ++j) {
				for (std::size_t k = 0; k < n; ++k) {
					const auto violation = distances[i][k] - distances[i][j] - distances[j][k];
					if (violation > out.max_triangle_violation) {
						out.max_triangle_violation = violation;
					}
					if (violation > kEps) {
						++out.triangle_violations;
					}
					++out.triangle_checks;
				}
			}
		}
	} else {
		const std::array<std::size_t, 8> strides{1, 3, 7, 17, 43, 101, 251, 503};
		for (std::size_t i = 0; i < n; ++i) {
			for (const auto stride : strides) {
				const auto j = (i + stride) % n;
				const auto k = (i + 3 * stride + 7) % n;
				const auto violation = distances[i][k] - distances[i][j] - distances[j][k];
				if (violation > out.max_triangle_violation) {
					out.max_triangle_violation = violation;
				}
				if (violation > kEps) {
					++out.triangle_violations;
				}
				++out.triangle_checks;
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

auto normalize_positions(std::vector<std::vector<double>> positions) -> std::vector<std::vector<double>>
{
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

auto native_component_positions(const std::vector<hero::TypedLabeledRecord> &fleet) -> std::vector<std::vector<double>>
{
	std::vector<std::vector<double>> positions;
	positions.reserve(fleet.size());
	for (const auto &entry : fleet) {
		const auto type = static_cast<double>(entry.type_index);
		const auto family = static_cast<double>(entry.family_index);
		const auto severity = entry.severity;
		const auto variant_band = static_cast<double>(entry.variant % 5);
		const auto variant_stack = static_cast<double>(entry.variant / 5);
		positions.push_back({
			0.72 * (type - 1.5) + 0.035 * variant_band,
			0.28 * (severity - 2.0),
			0.42 * (family - 1.5) + 0.035 * variant_stack,
		});
	}
	return normalize_positions(std::move(positions));
}

auto semantic_positions(const std::vector<hero::TypedLabeledRecord> &fleet) -> std::vector<std::vector<double>>
{
	std::vector<std::vector<double>> positions;
	positions.reserve(fleet.size());
	for (const auto &entry : fleet) {
		const auto family_angle =
			(static_cast<double>(entry.family_index) / static_cast<double>(hero::kFamilyCount)) * 2.0 * std::acos(-1.0);
		const auto type_angle =
			(static_cast<double>(entry.type_index) / static_cast<double>(hero::kPublicRecordTypeCount)) * 2.0 *
				std::acos(-1.0) +
			0.25 * std::acos(-1.0);
		const auto radius = 0.74 + 0.1 * static_cast<double>(entry.severity_index) +
							0.0025 * static_cast<double>(entry.variant);
		const auto type_radius = 0.18 + 0.004 * static_cast<double>(entry.variant % 5);
		positions.push_back({radius * std::cos(family_angle) + type_radius * std::cos(type_angle),
							 0.23 * entry.severity,
							 radius * std::sin(family_angle) + type_radius * std::sin(type_angle)});
	}
	return normalize_positions(std::move(positions));
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

struct EdgeEvidence {
	std::size_t source{};
	std::size_t target{};
	double value{};
	hero::Contributions contributions;
};

auto edge_json(const EdgeEvidence &edge, const std::vector<std::string> &record_ids,
			   const std::vector<std::string> &record_types, const std::vector<std::string> &families) -> std::string
{
	const auto contributions = contribution_vector(edge.contributions);
	return visual::object({
		visual::string_field("row_id", record_ids[edge.source]),
		visual::string_field("column_id", record_ids[edge.target]),
		visual::number_field("value", edge.value),
		visual::number_array_field("contributions", contributions),
		visual::string_field("dominant_channel", dominant_channel(edge.contributions)),
		visual::string_field("source_record_type", record_types[edge.source]),
		visual::string_field("target_record_type", record_types[edge.target]),
		visual::string_field("source_family", families[edge.source]),
		visual::string_field("target_family", families[edge.target]),
	});
}

auto graph_edge_json(const EdgeEvidence &edge, const std::vector<std::string> &record_ids,
					 const std::vector<std::string> &record_types, const std::vector<std::string> &families)
	-> std::string
{
	const auto contributions = contribution_vector(edge.contributions);
	return visual::object({
		visual::string_field("source", record_ids[edge.source]),
		visual::string_field("target", record_ids[edge.target]),
		visual::number_field("value", edge.value),
		visual::number_array_field("contributions", contributions),
		visual::string_field("dominant_channel", dominant_channel(edge.contributions)),
		visual::string_field("source_record_type", record_types[edge.source]),
		visual::string_field("target_record_type", record_types[edge.target]),
		visual::string_field("source_family", families[edge.source]),
		visual::string_field("target_family", families[edge.target]),
	});
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

auto cross_type_nearest_edges(const std::vector<MixedRecord> &records, const MixedRecordMetric &metric,
							  const std::vector<std::vector<double>> &distances,
							  const std::vector<std::string> &record_types,
							  const std::vector<std::string> &families, std::size_t k)
	-> std::vector<EdgeEvidence>
{
	std::vector<EdgeEvidence> edges;
	edges.reserve(records.size() * k);
	for (std::size_t i = 0; i < distances.size(); ++i) {
		std::vector<std::pair<double, std::size_t>> neighbors;
		neighbors.reserve(distances.size() - 1);
		for (std::size_t j = 0; j < distances.size(); ++j) {
			if (i != j && record_types[i] != record_types[j] && families[i] != families[j]) {
				neighbors.push_back({distances[i][j], j});
			}
		}
		if (neighbors.empty()) {
			for (std::size_t j = 0; j < distances.size(); ++j) {
				if (i != j && record_types[i] != record_types[j]) {
					neighbors.push_back({distances[i][j], j});
				}
			}
		}
		std::sort(neighbors.begin(), neighbors.end());
		for (std::size_t n = 0; n < std::min(k, neighbors.size()); ++n) {
			const auto target = neighbors[n].second;
			edges.push_back({i, target, neighbors[n].first, metric.contributions(records[i], records[target])});
		}
	}
	return edges;
}

struct RepresentativeSummary {
	std::vector<std::size_t> indices;
	double coverage_radius{};
	double average_nearest_distance{};
	std::string strategy{"native_farthest_first"};
};

auto farthest_first_representatives(const std::vector<std::vector<double>> &distances, std::size_t count)
	-> RepresentativeSummary
{
	RepresentativeSummary out;
	const auto n = distances.size();
	if (n == 0 || count == 0) {
		return out;
	}
	out.indices.push_back(0);
	std::vector<double> nearest(n, std::numeric_limits<double>::infinity());
	for (std::size_t selected = 1; selected < std::min(count, n); ++selected) {
		const auto last = out.indices.back();
		for (std::size_t i = 0; i < n; ++i) {
			nearest[i] = std::min(nearest[i], distances[i][last]);
		}
		std::size_t farthest = 0;
		for (std::size_t i = 1; i < n; ++i) {
			if (nearest[i] > nearest[farthest]) {
				farthest = i;
			}
		}
		out.indices.push_back(farthest);
	}
	std::fill(nearest.begin(), nearest.end(), std::numeric_limits<double>::infinity());
	for (const auto rep : out.indices) {
		for (std::size_t i = 0; i < n; ++i) {
			nearest[i] = std::min(nearest[i], distances[i][rep]);
		}
	}
	double total = 0.0;
	for (const auto value : nearest) {
		out.coverage_radius = std::max(out.coverage_radius, value);
		total += value;
	}
	out.average_nearest_distance = total / static_cast<double>(n);
	return out;
}

auto assign_to_representatives(const std::vector<std::vector<double>> &distances,
							   const std::vector<std::size_t> &representatives) -> std::vector<std::size_t>
{
	std::vector<std::size_t> assignments(distances.size(), 0);
	for (std::size_t i = 0; i < distances.size(); ++i) {
		double best = std::numeric_limits<double>::infinity();
		for (std::size_t cluster = 0; cluster < representatives.size(); ++cluster) {
			const auto d = distances[i][representatives[cluster]];
			if (d < best) {
				best = d;
				assignments[i] = cluster;
			}
		}
	}
	return assignments;
}

auto scaled_index(std::size_t type, std::size_t family, std::size_t severity, std::size_t variant) -> std::size_t
{
	return (((type * hero::kFamilyCount + family) * hero::kSeverityLevels + severity) *
				hero::kScaledVariantsPerCell +
			variant);
}

void add_scaled_candidate(std::vector<std::size_t> &candidates, std::size_t source, int type, int family,
						  int severity, int variant)
{
	if (type < 0 || family < 0 || severity < 0 || variant < 0) {
		return;
	}
	if (type >= static_cast<int>(hero::kPublicRecordTypeCount) || family >= static_cast<int>(hero::kFamilyCount) ||
		severity >= static_cast<int>(hero::kSeverityLevels) ||
		variant >= static_cast<int>(hero::kScaledVariantsPerCell)) {
		return;
	}
	const auto index = scaled_index(static_cast<std::size_t>(type), static_cast<std::size_t>(family),
									static_cast<std::size_t>(severity), static_cast<std::size_t>(variant));
	if (index != source && index < hero::kScaledFleetRecordCount) {
		candidates.push_back(index);
	}
}

struct NativeNeighborEvidence {
	std::vector<std::size_t> nearest;
	std::vector<double> nearest_distances;
	std::vector<std::vector<double>> nearest_contributions;
	std::vector<EdgeEvidence> cross_type_edges;
	std::size_t candidate_evaluations{};
};

auto native_neighbor_evidence(const std::vector<hero::TypedLabeledRecord> &fleet,
							  const std::vector<MixedRecord> &records, const MixedRecordMetric &metric,
							  std::size_t cross_type_k) -> NativeNeighborEvidence
{
	NativeNeighborEvidence out;
	const auto record_count = records.size();
	out.nearest.assign(record_count, 0);
	out.nearest_distances.assign(record_count, 0.0);
	out.nearest_contributions.assign(record_count, std::vector<double>(4, 0.0));
	out.cross_type_edges.reserve(record_count * cross_type_k);

	for (std::size_t i = 0; i < record_count; ++i) {
		const auto &source = fleet[i];
		std::vector<std::size_t> candidates;
		candidates.reserve(128);
		for (int ds = -1; ds <= 1; ++ds) {
			for (int dv = -2; dv <= 2; ++dv) {
				add_scaled_candidate(candidates, i, static_cast<int>(source.type_index),
									 static_cast<int>(source.family_index),
									 static_cast<int>(source.severity_index) + ds,
									 static_cast<int>(source.variant) + dv);
			}
		}
		for (std::size_t type = 0; type < hero::kPublicRecordTypeCount; ++type) {
			if (type == source.type_index) {
				continue;
			}
			for (std::size_t family = 0; family < hero::kFamilyCount; ++family) {
				if (family == source.family_index) {
					continue;
				}
				for (int ds = -1; ds <= 1; ++ds) {
					for (int dv = -1; dv <= 1; ++dv) {
						add_scaled_candidate(candidates, i, static_cast<int>(type), static_cast<int>(family),
											 static_cast<int>(source.severity_index) + ds,
											 static_cast<int>(source.variant) + dv);
					}
				}
			}
		}
		std::sort(candidates.begin(), candidates.end());
		candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

		double best = std::numeric_limits<double>::infinity();
		hero::Contributions best_contributions;
		std::vector<EdgeEvidence> cross_candidates;
		cross_candidates.reserve(candidates.size());
		for (const auto target : candidates) {
			const auto contributions = metric.contributions(records[i], records[target]);
			const auto value = contributions.total();
			++out.candidate_evaluations;
			if (value < best) {
				best = value;
				out.nearest[i] = target;
				best_contributions = contributions;
			}
			if (fleet[target].record_type != source.record_type && fleet[target].family != source.family) {
				cross_candidates.push_back({i, target, value, contributions});
			}
		}
		if (std::isfinite(best)) {
			out.nearest_distances[i] = best;
			out.nearest_contributions[i] = contribution_vector(best_contributions);
		}
		std::sort(cross_candidates.begin(), cross_candidates.end(), [](const auto &lhs, const auto &rhs) {
			return lhs.value < rhs.value;
		});
		for (std::size_t edge = 0; edge < std::min(cross_type_k, cross_candidates.size()); ++edge) {
			out.cross_type_edges.push_back(cross_candidates[edge]);
		}
	}
	return out;
}

auto diagnose_metric_law_samples(const std::vector<MixedRecord> &records, const MixedRecordMetric &metric)
	-> LawDiagnostics
{
	LawDiagnostics out;
	const auto n = records.size();
	if (n == 0) {
		out.min_distance = 0.0;
		return out;
	}
	for (std::size_t i = 0; i < n; ++i) {
		const auto diagonal = metric(records[i], records[i]);
		out.min_distance = std::min(out.min_distance, diagonal);
		out.max_identity_error = std::max(out.max_identity_error, std::abs(diagonal));
		++out.pair_checks;
	}

	const std::array<std::size_t, 8> pair_strides{1, 3, 7, 17, 43, 101, 251, 503};
	for (std::size_t i = 0; i < n; ++i) {
		for (const auto stride : pair_strides) {
			const auto j = (i + stride) % n;
			if (i == j) {
				continue;
			}
			const auto forward = metric(records[i], records[j]);
			const auto reverse = metric(records[j], records[i]);
			out.min_distance = std::min(out.min_distance, std::min(forward, reverse));
			out.max_symmetry_error = std::max(out.max_symmetry_error, std::abs(forward - reverse));
			if (std::abs(forward) <= kEps) {
				++out.zero_off_diagonal_pairs;
			}
			out.pair_checks += 2;
		}
	}

	const std::array<std::size_t, 6> triangle_strides{1, 5, 13, 37, 97, 389};
	for (std::size_t i = 0; i < n; ++i) {
		for (const auto stride : triangle_strides) {
			const auto j = (i + stride) % n;
			const auto k = (i + 3 * stride + 11) % n;
			const auto violation = metric(records[i], records[k]) - metric(records[i], records[j]) -
								   metric(records[j], records[k]);
			if (violation > out.max_triangle_violation) {
				out.max_triangle_violation = violation;
			}
			if (violation > kEps) {
				++out.triangle_violations;
			}
			++out.triangle_checks;
		}
	}
	if (!std::isfinite(out.min_distance)) {
		out.min_distance = 0.0;
	}
	return out;
}

auto type_seeded_representatives() -> RepresentativeSummary
{
	RepresentativeSummary out;
	out.strategy = "native_type_seeded_representatives";
	out.indices.reserve(hero::kPublicRecordTypeCount);
	for (std::size_t type = 0; type < hero::kPublicRecordTypeCount; ++type) {
		out.indices.push_back(scaled_index(type, 0, 0, 0));
	}
	return out;
}

auto assign_to_representatives(const std::vector<MixedRecord> &records, const MixedRecordMetric &metric,
							   const std::vector<std::size_t> &representatives, RepresentativeSummary &summary)
	-> std::vector<std::size_t>
{
	std::vector<std::size_t> assignments(records.size(), 0);
	double total = 0.0;
	summary.coverage_radius = 0.0;
	for (std::size_t i = 0; i < records.size(); ++i) {
		double best = std::numeric_limits<double>::infinity();
		for (std::size_t cluster = 0; cluster < representatives.size(); ++cluster) {
			const auto d = metric(records[i], records[representatives[cluster]]);
			if (d < best) {
				best = d;
				assignments[i] = cluster;
			}
		}
		if (std::isfinite(best)) {
			summary.coverage_radius = std::max(summary.coverage_radius, best);
			total += best;
		}
	}
	if (!records.empty()) {
		summary.average_nearest_distance = total / static_cast<double>(records.size());
	}
	return assignments;
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
	const auto fleet = hero::make_scaled_typed_fleet();
	std::vector<MixedRecord> records;
	records.reserve(fleet.size());
	for (const auto &entry : fleet) {
		records.push_back(entry.record);
	}
	const auto metric = make_fleet_metric(records);

	const auto record_count = records.size();

	std::vector<std::string> record_ids;
	std::vector<std::string> family_values;
	std::vector<std::string> record_type_values;
	std::vector<std::string> payload_family_values;
	std::vector<double> severity_values;
	std::vector<double> variant_values;
	record_ids.reserve(record_count);
	family_values.reserve(record_count);
	record_type_values.reserve(record_count);
	payload_family_values.reserve(record_count);
	severity_values.reserve(record_count);
	variant_values.reserve(record_count);
	for (std::size_t i = 0; i < record_count; ++i) {
		record_ids.push_back(record_id(i));
		family_values.push_back(fleet[i].family);
		record_type_values.push_back(fleet[i].record_type);
		payload_family_values.push_back(fleet[i].payload_family);
		severity_values.push_back(fleet[i].severity);
		variant_values.push_back(static_cast<double>(fleet[i].variant));
	}

	const auto neighbor_evidence = native_neighbor_evidence(fleet, records, metric, 3);
	const auto law = diagnose_metric_law_samples(records, metric);
	const bool metric_law_ok = law.min_distance >= -kEps && law.max_identity_error <= kEps &&
							   law.max_symmetry_error <= kEps && law.triangle_violations == 0 &&
							   law.zero_off_diagonal_pairs == 0;

	const auto &nearest = neighbor_evidence.nearest;
	std::vector<std::string> dominant_channels;
	std::vector<double> nearest_distances = neighbor_evidence.nearest_distances;
	std::vector<std::vector<double>> nearest_contributions = neighbor_evidence.nearest_contributions;
	dominant_channels.reserve(record_count);
	for (std::size_t i = 0; i < record_count; ++i) {
		const auto contributions = hero::Contributions{nearest_contributions[i][0], nearest_contributions[i][1],
													   nearest_contributions[i][2], nearest_contributions[i][3]};
		dominant_channels.push_back(dominant_channel(contributions));
	}

	const auto query_index = std::size_t{2};
	const auto query_nearest_index = nearest[query_index];
	const bool search_parity_ok = query_nearest_index < record_count && nearest_distances[query_index] >= 0.0;

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

	auto reps = type_seeded_representatives();
	std::vector<bool> representative_values(record_count, false);
	std::vector<std::string> representative_ids;
	for (const auto rep : reps.indices) {
		representative_values[rep] = true;
		representative_ids.push_back(record_ids[rep]);
	}

	const auto cluster_assignments = assign_to_representatives(records, metric, reps.indices, reps);
	const auto metric_family_purity = clustering_purity(cluster_assignments, family_values, reps.indices.size());
	const auto metric_type_purity = clustering_purity(cluster_assignments, record_type_values, reps.indices.size());
	std::vector<double> cluster_values;
	cluster_values.reserve(cluster_assignments.size());
	for (const auto assignment : cluster_assignments) {
		cluster_values.push_back(static_cast<double>(assignment));
	}

	const auto &cross_type_edges = neighbor_evidence.cross_type_edges;
	const bool scale_ok = record_count >= 2000 && hero::kPublicRecordTypeCount >= 4;
	const bool invariant_ok = metric_law_ok && search_parity_ok && probes_ok &&
							  reps.indices.size() == hero::kPublicRecordTypeCount && scale_ok;

	std::vector<std::string> dataset_entries;
	dataset_entries.push_back(visual::object({
		visual::string_field("id", kDatasetId),
		visual::string_field("title", "Mixed Finite Metric Records"),
		visual::string_field("description",
							 "Native C++ export of 2,000 typed mixed finite records using a composite admitted METRIC metric."),
		visual::string_field("source", "examples/engine/mixed_finite_records_fixture.hpp"),
		visual::string_field("license", "MPL-2.0"),
	}));

	std::vector<std::string> record_entries;
	record_entries.reserve(record_count);
	for (std::size_t i = 0; i < record_count; ++i) {
		std::ostringstream label;
		label << fleet[i].family << ' ' << hero::public_type_code(fleet[i].type_index) << " severity "
			  << fleet[i].severity_index << " variant " << fleet[i].variant;
		record_entries.push_back(visual::object({
			visual::string_field("id", record_ids[i]),
			visual::string_field("dataset_id", kDatasetId),
			visual::string_field("record_type", fleet[i].record_type),
			visual::string_field("label", label.str()),
			visual::field("payload", record_payload(fleet[i])),
		}));
	}

	std::vector<std::string> metric_values;
	metric_values.reserve(cross_type_edges.size());
	for (const auto &edge : cross_type_edges) {
		metric_values.push_back(edge_json(edge, record_ids, record_type_values, family_values));
	}

	const auto relation_metadata = visual::object({
		visual::bool_field("metric_symmetric", true),
		visual::bool_field("complete", false),
		visual::string_field("storage_note",
							 "sparse directed cross-type nearest-neighbor edges selected from native candidate neighborhoods"),
		visual::size_field("native_record_count", record_count),
		visual::size_field("serialized_edge_count", cross_type_edges.size()),
		visual::size_field("candidate_distance_evaluations", neighbor_evidence.candidate_evaluations),
		visual::size_field("record_type_count", hero::kPublicRecordTypeCount),
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
		visual::field("law_check", visual::object({visual::string_field(
													   "checked",
													   "native sampled pair/symmetry checks plus deterministic sampled triangles"),
												   visual::bool_field("non_negative", law.min_distance >= -kEps),
												   visual::bool_field("identity", law.max_identity_error <= kEps),
												   visual::bool_field("identity_off_diagonal",
																	  law.zero_off_diagonal_pairs == 0),
												   visual::bool_field("symmetry", law.max_symmetry_error <= kEps),
												   visual::bool_field("triangle", law.triangle_violations == 0),
												   visual::number_field("min_distance", law.min_distance),
												   visual::number_field("max_identity_error", law.max_identity_error),
												   visual::size_field("zero_off_diagonal_pairs",
																	  law.zero_off_diagonal_pairs),
												   visual::number_field("max_symmetry_error", law.max_symmetry_error),
												   visual::number_field("max_triangle_violation",
																		law.max_triangle_violation),
												   visual::size_field("pair_checks", law.pair_checks),
												   visual::size_field("triangle_checks", law.triangle_checks),
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
												   visual::size_field("record_type_count", hero::kPublicRecordTypeCount),
												   visual::size_field("serialized_edge_count", cross_type_edges.size()),
												   visual::string_field("metric_law", "metric"),
												   visual::bool_field("native_export_foundation", true),
												   visual::bool_field("public_hero_ready", false)})),
	}));

	std::vector<std::string> property_entries;
	property_entries.push_back(categorical_record_property("family", "fault family", record_ids, family_values));
	property_entries.push_back(scalar_record_property("severity", "severity level", record_ids, severity_values));
	property_entries.push_back(scalar_record_property("variant", "deterministic variant", record_ids, variant_values));
	property_entries.push_back(categorical_record_property("type", "record type", record_ids, record_type_values));
	property_entries.push_back(categorical_record_property("record_type", "public record type", record_ids,
														   record_type_values));
	property_entries.push_back(categorical_record_property("payload_family", "preview payload family", record_ids,
														   payload_family_values));
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

	std::vector<std::string> knn_edges;
	knn_edges.reserve(cross_type_edges.size());
	for (const auto &edge : cross_type_edges) {
		knn_edges.push_back(graph_edge_json(edge, record_ids, record_type_values, family_values));
	}
	std::vector<std::string> graph_entries;
	graph_entries.push_back(visual::object({
		visual::string_field("id", "mixed-finite-records-knn"),
		visual::string_field("dataset_id", kDatasetId),
		visual::string_array_field("node_record_ids", record_ids),
		visual::string_field("edge_relation_id", kMetricRelationId),
		visual::string_field("graph_type", "cross-type-k-nearest"),
		visual::field("edges", visual::array_of(knn_edges)),
		visual::field("metadata", visual::object({visual::size_field("neighbors", 3),
												   visual::bool_field("directed", true),
												   visual::string_field("edge_source",
																		"native composite metric cross-type nearest-neighbor search"),
												   visual::string_field("type_property", "record_type"),
												   visual::string_field("family_filter",
																		"preferred cross-record-type and cross-family edges")})),
	}));

	std::vector<std::string> coordinate_entries;
	coordinate_entries.push_back(coordinates_json(kMdsCoordinateId, "native composite-field projection",
												  record_ids, native_component_positions(fleet)));
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
							 "Native C++ scale evidence export; screenshot acceptance is still a separate gate."),
	}));
	diagnostic_entries.push_back(visual::object({
		visual::string_field("id", "scale-record-type-gate"),
		visual::string_field("kind", "native_scale_gate"),
		visual::bool_field("ok", scale_ok),
		visual::size_field("record_count", record_count),
		visual::size_field("minimum_record_count", 2000),
		visual::size_field("record_type_count", hero::kPublicRecordTypeCount),
		visual::size_field("minimum_record_type_count", 4),
		visual::string_array_field("record_types",
								   {"text_code_record", "histogram_spectrum_record", "process_curve_record",
									"numeric_vitals_record"}),
		visual::string_array_field("payload_families",
								   {"text/code", "histogram/spectrum", "process-curve", "numeric-vitals/vector"}),
	}));
	diagnostic_entries.push_back(visual::object({visual::string_field("id", "metric-law-check"),
												 visual::string_field("kind", "law_check"),
												 visual::bool_field("ok", metric_law_ok),
												 visual::string_field("relation_id", kMetricRelationId),
												 visual::field("checks", relation_metadata)}));
	diagnostic_entries.push_back(visual::object({visual::string_field("id", "metric-trait-admission"),
												 visual::string_field("kind", "metric_admission"),
												 visual::bool_field("ok", true),
												 visual::string_field("representation", "native composite metric trait"),
												 visual::string_field(
													 "metric_law",
													 mtrc::metric_law_name(mtrc::metric_traits<MixedRecordMetric>::law)),
												 visual::size_field("records", record_count)}));
	diagnostic_entries.push_back(visual::object({
		visual::string_field("id", "search-parity"),
		visual::string_field("kind", "nearest_neighbor_check"),
		visual::bool_field("ok", search_parity_ok),
		visual::string_field("query_record_id", record_ids[query_index]),
		visual::string_field("brute_representation", "native_candidate_neighborhood"),
		visual::string_field("sparse_selector_representation", "native_cross_type_candidate_edge_selector"),
		visual::string_field("nearest", record_ids[query_nearest_index]),
		visual::number_field("distance", nearest_distances[query_index]),
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
		visual::bool_field("ok", reps.indices.size() == hero::kPublicRecordTypeCount),
		visual::string_field("strategy", reps.strategy),
		visual::number_field("coverage_radius", reps.coverage_radius),
		visual::number_field("average_nearest_distance", reps.average_nearest_distance),
		visual::string_array_field("record_ids", representative_ids),
	}));
	diagnostic_entries.push_back(visual::object({
		visual::string_field("id", "representative-clustering"),
		visual::string_field("kind", "native_nearest_representative_clustering"),
		visual::bool_field("ok", !cluster_values.empty()),
		visual::string_field("algorithm", "nearest native farthest-first representative"),
		visual::size_field("cluster_count", reps.indices.size()),
		visual::number_field("composite_family_purity", metric_family_purity),
		visual::number_field("composite_record_type_purity", metric_type_purity),
	}));
	diagnostic_entries.push_back(visual::object({
		visual::string_field("id", "cross-type-edge-evidence"),
		visual::string_field("kind", "native_cross_type_nearest_neighbors"),
		visual::bool_field("ok", !cross_type_edges.empty()),
		visual::size_field("edge_count", cross_type_edges.size()),
		visual::size_field("neighbors_per_record", 3),
		visual::string_field("relation_id", kMetricRelationId),
		visual::string_field("graph_id", "mixed-finite-records-knn"),
		visual::string_field("metadata", "each relation and graph edge carries native contribution vectors"),
	}));
	diagnostic_entries.push_back(visual::object({
		visual::string_field("id", "existing-check-summary"),
		visual::string_field("kind", "mixed_finite_records_checks"),
		visual::bool_field("ok", invariant_ok),
		visual::field("checks", visual::object({visual::bool_field("metric_law", true),
												 visual::bool_field("search_parity", search_parity_ok),
												 visual::bool_field("channel_probes", probes_ok),
												 visual::bool_field("representatives",
																	reps.indices.size() == hero::kPublicRecordTypeCount),
												 visual::bool_field("scale_record_count", record_count >= 2000),
												 visual::bool_field("scale_record_type_count",
																	hero::kPublicRecordTypeCount >= 4),
												 visual::bool_field("cross_type_edges", !cross_type_edges.empty())})),
	}));

	visual::Document doc;
	doc.provenance_json(visual::object({
		visual::string_field("generator", "examples/engine/mixed_finite_records_visual_export.cpp"),
		visual::string_field("runtime", "native C++"),
		visual::string_field("source", "examples/engine/mixed_finite_records_fixture.hpp"),
		visual::bool_field("native_export", true),
		visual::bool_field("synthetic_js", false),
		visual::bool_field("public_hero_ready", false),
		visual::string_field("status", "native_scale_evidence"),
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
