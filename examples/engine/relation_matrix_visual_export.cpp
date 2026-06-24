// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Native metric.visual.v1 exporter for a relation-matrix foundation.
//
// The evidence is computed entirely in C++ from the process-curve metric shape
// used by examples/engine/process_curves_space.cpp. It emits records ordered by
// family block, a complete symmetric metric relation, categorical block/order
// properties, deterministic visualization coordinates, and a k-nearest graph.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "mtrc_visual.hpp"

namespace {

namespace visual = mtrc::visual;

constexpr const char *kDatasetId = "process-curve-relation-matrix";
constexpr const char *kRelationId = "process-curve-aligned-metric";
constexpr const char *kSpaceId = "process-curve-finite-space";
constexpr const char *kCoordinateId = "process-curve-block-layout-3d";
constexpr const char *kGraphId = "process-curve-knn";
constexpr std::size_t kVariantsPerFamily = 26;
constexpr std::size_t kNearestNeighbors = 5;
constexpr double kPi = 3.14159265358979323846;

using Curve = std::vector<double>;
using Matrix = std::vector<std::vector<double>>;

struct FamilyTemplate {
	std::string id;
	std::string label;
	Curve base;
};

struct Record {
	std::string id;
	std::string label;
	std::string family_id;
	std::string family_label;
	std::size_t family_index{};
	std::size_t variant_index{};
	Curve curve;
};

struct MetricLawDiagnostics {
	double diagonal_max_abs{};
	double symmetry_max_abs{};
	double triangle_max_violation{};
	double min_nonzero_distance{};
	double max_distance{};
	double mean_distance{};
	std::size_t pair_count{};
	std::size_t triangle_triplets{};
	bool finite{true};
};

struct Neighbor {
	std::size_t index{};
	double distance{};
};

auto record_id(std::size_t index) -> std::string
{
	std::ostringstream out;
	out << "pc-";
	if (index < 100) {
		out << '0';
	}
	if (index < 10) {
		out << '0';
	}
	out << index;
	return out.str();
}

auto family_templates() -> std::vector<FamilyTemplate>
{
	return {
		{"normal_reference", "normal reference", {0, 0, 1, 1, 1, 2, 3, 3}},
		{"flat_hold", "flat hold", {0, 0, 0, 0, 1, 1, 1, 1, 1, 1}},
		{"late_ramp", "late ramp", {0, 0, 0, 1, 1, 1, 2, 3, 3}},
		{"spike", "spike", {0, 0, 1, 7, 1, 2, 3, 3}},
		{"early_ramp", "early ramp", {0, 1, 1, 1, 2, 3, 3, 3}},
	};
}

auto variant_curve(const FamilyTemplate &family, std::size_t family_index, std::size_t variant_index) -> Curve
{
	Curve curve;
	curve.reserve(family.base.size());

	const double centered = static_cast<double>(variant_index) -
							(static_cast<double>(kVariantsPerFamily) - 1.0) * 0.5;
	const double scale = 1.0 + 0.012 * centered;
	const double shift = 0.018 * static_cast<double>((variant_index % 5) + 1);
	const double slope = 0.014 * static_cast<double>((variant_index % 7)) - 0.042;

	for (std::size_t sample = 0; sample < family.base.size(); ++sample) {
		const double t = family.base.size() <= 1
							 ? 0.0
							 : static_cast<double>(sample) / static_cast<double>(family.base.size() - 1);
		const double ripple =
			0.035 * std::sin((static_cast<double>(family_index) + 1.0) *
							 (static_cast<double>(variant_index) + 1.0) * (static_cast<double>(sample) + 1.0) *
							 0.173);
		double value = family.base[sample] * scale + shift + slope * t + ripple;
		if (family.id == "flat_hold" && sample >= 4) {
			value += 0.018 * static_cast<double>(variant_index % 4);
		}
		if (family.id == "late_ramp" && sample < 3) {
			value -= 0.012 * static_cast<double>(variant_index % 6);
		}
		if (family.id == "early_ramp" && sample <= 2) {
			value += 0.015 * static_cast<double>(variant_index % 6);
		}
		if (family.id == "spike" && sample == 3) {
			value += 0.09 * static_cast<double>((variant_index % 9)) - 0.36;
		}
		curve.push_back(std::max(0.0, value));
	}

	if (variant_index % 4 == 1) {
		curve.insert(curve.begin(), curve.front());
	} else if (variant_index % 4 == 2) {
		curve.push_back(curve.back());
	} else if (variant_index % 4 == 3 && curve.size() > 7) {
		const auto insert_at = curve.begin() + static_cast<std::ptrdiff_t>(curve.size() / 2);
		curve.insert(insert_at, curve[curve.size() / 2]);
	}
	curve.back() += 0.001 * static_cast<double>(variant_index + 1);
	return curve;
}

auto make_records() -> std::vector<Record>
{
	const auto families = family_templates();
	std::vector<Record> records;
	records.reserve(families.size() * kVariantsPerFamily);

	for (std::size_t family_index = 0; family_index < families.size(); ++family_index) {
		for (std::size_t variant = 0; variant < kVariantsPerFamily; ++variant) {
			const std::size_t index = records.size();
			std::ostringstream label;
			label << families[family_index].label << " / variant " << variant;
			records.push_back({record_id(index),
							   label.str(),
							   families[family_index].id,
							   families[family_index].label,
							   family_index,
							   variant,
							   variant_curve(families[family_index], family_index, variant)});
		}
	}

	return records;
}

struct AlignedCurveDistance {
	double gap_cost{2.0};

	auto operator()(const Curve &lhs, const Curve &rhs) const -> double
	{
		std::vector<double> previous(rhs.size() + 1, 0.0);
		for (std::size_t index = 0; index < previous.size(); ++index) {
			previous[index] = static_cast<double>(index) * gap_cost;
		}

		for (std::size_t lhs_index = 1; lhs_index <= lhs.size(); ++lhs_index) {
			std::vector<double> current(rhs.size() + 1, 0.0);
			current[0] = static_cast<double>(lhs_index) * gap_cost;
			for (std::size_t rhs_index = 1; rhs_index <= rhs.size(); ++rhs_index) {
				const double substitution =
					previous[rhs_index - 1] +
					std::min(std::abs(lhs[lhs_index - 1] - rhs[rhs_index - 1]), 2.0 * gap_cost);
				const double deletion = previous[rhs_index] + gap_cost;
				const double insertion = current[rhs_index - 1] + gap_cost;
				current[rhs_index] = std::min({substitution, deletion, insertion});
			}
			previous = std::move(current);
		}

		return previous.back();
	}
};

auto distance_matrix(const std::vector<Record> &records) -> Matrix
{
	AlignedCurveDistance metric;
	Matrix distances(records.size(), std::vector<double>(records.size(), 0.0));
	for (std::size_t row = 0; row < records.size(); ++row) {
		for (std::size_t column = row + 1; column < records.size(); ++column) {
			const double value = metric(records[row].curve, records[column].curve);
			distances[row][column] = value;
			distances[column][row] = value;
		}
	}
	return distances;
}

auto diagnose_metric_law(const Matrix &distances) -> MetricLawDiagnostics
{
	MetricLawDiagnostics diagnostics;
	diagnostics.min_nonzero_distance = std::numeric_limits<double>::infinity();
	const std::size_t count = distances.size();

	for (std::size_t row = 0; row < count; ++row) {
		diagnostics.diagonal_max_abs = std::max(diagnostics.diagonal_max_abs, std::abs(distances[row][row]));
		diagnostics.finite = diagnostics.finite && std::isfinite(distances[row][row]);
		for (std::size_t column = row + 1; column < count; ++column) {
			const double distance = distances[row][column];
			diagnostics.symmetry_max_abs =
				std::max(diagnostics.symmetry_max_abs, std::abs(distance - distances[column][row]));
			diagnostics.min_nonzero_distance = std::min(diagnostics.min_nonzero_distance, distance);
			diagnostics.max_distance = std::max(diagnostics.max_distance, distance);
			diagnostics.mean_distance += distance;
			diagnostics.finite = diagnostics.finite && std::isfinite(distance) && std::isfinite(distances[column][row]);
			++diagnostics.pair_count;
		}
	}

	if (diagnostics.pair_count != 0) {
		diagnostics.mean_distance /= static_cast<double>(diagnostics.pair_count);
	}
	if (!std::isfinite(diagnostics.min_nonzero_distance)) {
		diagnostics.min_nonzero_distance = 0.0;
	}

	for (std::size_t i = 0; i < count; ++i) {
		for (std::size_t j = 0; j < count; ++j) {
			for (std::size_t k = 0; k < count; ++k) {
				const double violation = distances[i][k] - distances[i][j] - distances[j][k];
				diagnostics.triangle_max_violation = std::max(diagnostics.triangle_max_violation, violation);
				++diagnostics.triangle_triplets;
			}
		}
	}

	return diagnostics;
}

auto curve_mean(const Curve &curve) -> double
{
	if (curve.empty()) {
		return 0.0;
	}
	return std::accumulate(curve.begin(), curve.end(), 0.0) / static_cast<double>(curve.size());
}

auto curve_peak(const Curve &curve) -> double
{
	return curve.empty() ? 0.0 : *std::max_element(curve.begin(), curve.end());
}

auto curve_total_variation(const Curve &curve) -> double
{
	double total = 0.0;
	for (std::size_t i = 1; i < curve.size(); ++i) {
		total += std::abs(curve[i] - curve[i - 1]);
	}
	return total;
}

auto record_ids(const std::vector<Record> &records) -> std::vector<std::string>
{
	std::vector<std::string> ids;
	ids.reserve(records.size());
	for (const auto &record : records) {
		ids.push_back(record.id);
	}
	return ids;
}

auto block_ranges_json() -> std::string
{
	const auto families = family_templates();
	std::vector<std::string> ranges;
	ranges.reserve(families.size());
	for (std::size_t family_index = 0; family_index < families.size(); ++family_index) {
		const std::size_t start = family_index * kVariantsPerFamily;
		ranges.push_back(visual::object({visual::string_field("family", families[family_index].id),
										 visual::string_field("block_label", families[family_index].label),
										 visual::size_field("start", start),
										 visual::size_field("end_exclusive", start + kVariantsPerFamily)}));
	}
	return visual::array_of(ranges);
}

auto record_payload_json(const Record &record) -> std::string
{
	return visual::object({
		visual::string_field("kind", "time_series"),
		visual::number_array_field("series", record.curve),
		visual::number_field("sample_rate_hz", 1.0),
		visual::string_field("family", record.family_id),
		visual::string_field("family_label", record.family_label),
		visual::size_field("variant_index", record.variant_index),
		visual::string_field("native_metric_source", "examples/engine/process_curves_space.cpp:AlignedCurveDistance"),
	});
}

auto records_json(const std::vector<Record> &records) -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(records.size());
	for (const auto &record : records) {
		entries.push_back(visual::object({visual::string_field("id", record.id),
										  visual::string_field("dataset_id", kDatasetId),
										  visual::string_field("record_type", "process_curve"),
										  visual::string_field("label", record.label),
										  visual::field("payload", record_payload_json(record))}));
	}
	return visual::array_of(entries);
}

auto relation_edges(const std::vector<Record> &records, const Matrix &distances) -> std::vector<visual::Edge>
{
	std::vector<visual::Edge> edges;
	edges.reserve((records.size() * (records.size() - 1)) / 2);
	for (std::size_t row = 0; row < records.size(); ++row) {
		for (std::size_t column = row + 1; column < records.size(); ++column) {
			edges.push_back({records[row].id, records[column].id, distances[row][column]});
		}
	}
	return edges;
}

auto relation_values_json(const std::vector<Record> &records, const Matrix &distances) -> std::string
{
	std::vector<std::string> values;
	values.reserve((records.size() * (records.size() - 1)) / 2);
	for (const auto &edge : relation_edges(records, distances)) {
		values.push_back(visual::object({visual::string_field("row_id", edge.row_id),
										 visual::string_field("column_id", edge.column_id),
										 visual::number_field("value", edge.value)}));
	}
	return visual::array_of(values);
}

auto metric_law_json(const MetricLawDiagnostics &law) -> std::string
{
	const bool diagonal_ok = law.diagonal_max_abs <= 1e-10;
	const bool symmetry_ok = law.symmetry_max_abs <= 1e-10;
	const bool triangle_ok = law.triangle_max_violation <= 1e-9;
	return visual::object({visual::string_field("checked", "full finite native C++ distance matrix"),
						   visual::string_field("operator", "AlignedCurveDistance"),
						   visual::bool_field("finite", law.finite),
						   visual::bool_field("diagonal_zero", diagonal_ok),
						   visual::bool_field("symmetric", symmetry_ok),
						   visual::bool_field("triangle", triangle_ok),
						   visual::number_field("diagonal_max_abs", law.diagonal_max_abs),
						   visual::number_field("symmetry_max_abs", law.symmetry_max_abs),
						   visual::number_field("triangle_max_violation", law.triangle_max_violation),
						   visual::number_field("min_nonzero_distance", law.min_nonzero_distance),
						   visual::number_field("max_distance", law.max_distance),
						   visual::number_field("mean_distance", law.mean_distance),
						   visual::size_field("pair_count", law.pair_count),
						   visual::size_field("triangle_triplets", law.triangle_triplets)});
}

auto relation_metadata_json(const MetricLawDiagnostics &law) -> std::string
{
	return visual::object({
		visual::bool_field("symmetric", true),
		visual::bool_field("complete_upper_triangle", true),
		visual::string_field("record_order", "contiguous categorical family blocks"),
		visual::string_field("storage_note", "upper-triangle pair entries for symmetric dense matrix rendering"),
		visual::field("block_ranges", block_ranges_json()),
		visual::field("law_check", metric_law_json(law)),
	});
}

auto relation_json(const std::vector<Record> &records, const Matrix &distances, const MetricLawDiagnostics &law)
	-> std::string
{
	return visual::object({visual::string_field("id", kRelationId),
						   visual::string_field("dataset_id", kDatasetId),
						   visual::string_field("name", "aligned process-curve metric"),
						   visual::string_field("relation_type", "metric"),
						   visual::string_field("value_type", "scalar"),
						   visual::string_array_field("record_ids", record_ids(records)),
						   visual::string_field("storage", "symmetric_dense_matrix"),
						   visual::field("values", relation_values_json(records, distances)),
						   visual::field("metadata", relation_metadata_json(law))});
}

auto categorical_values(const std::vector<Record> &records, const std::vector<std::string> &values)
	-> std::vector<visual::CategoricalValue>
{
	std::vector<visual::CategoricalValue> entries;
	entries.reserve(records.size());
	for (std::size_t i = 0; i < records.size(); ++i) {
		entries.push_back({records[i].id, values[i]});
	}
	return entries;
}

auto categorical_property_json(const std::string &id, const std::string &name, const std::vector<Record> &records,
							   const std::vector<std::string> &values, const std::string &metadata = "{}")
	-> std::string
{
	std::vector<std::string> entries;
	for (const auto &value : categorical_values(records, values)) {
		entries.push_back(visual::object({visual::string_field("record_id", value.record_id),
										  visual::string_field("value", value.value)}));
	}
	return visual::object({visual::string_field("id", id),
						   visual::string_field("dataset_id", kDatasetId),
						   visual::string_field("name", name),
						   visual::string_field("target_type", "record"),
						   visual::string_field("value_type", "categorical"),
						   visual::field("values", visual::array_of(entries)),
						   visual::field("metadata", metadata)});
}

auto scalar_values(const std::vector<Record> &records, const std::vector<double> &values)
	-> std::vector<visual::ScalarValue>
{
	std::vector<visual::ScalarValue> entries;
	entries.reserve(records.size());
	for (std::size_t i = 0; i < records.size(); ++i) {
		entries.push_back({records[i].id, values[i]});
	}
	return entries;
}

auto scalar_property_json(const std::string &id, const std::string &name, const std::vector<Record> &records,
						  const std::vector<double> &values, const std::string &metadata = "{}") -> std::string
{
	std::vector<std::string> entries;
	for (const auto &value : scalar_values(records, values)) {
		entries.push_back(visual::object({visual::string_field("record_id", value.record_id),
										  visual::number_field("value", value.value)}));
	}
	return visual::object({visual::string_field("id", id),
						   visual::string_field("dataset_id", kDatasetId),
						   visual::string_field("name", name),
						   visual::string_field("target_type", "record"),
						   visual::string_field("value_type", "scalar"),
						   visual::field("values", visual::array_of(entries)),
						   visual::field("metadata", metadata)});
}

auto properties_json(const std::vector<Record> &records) -> std::string
{
	std::vector<std::string> family_values;
	std::vector<std::string> block_order_values;
	std::vector<double> order_values;
	std::vector<double> within_block_values;
	std::vector<double> mean_values;
	std::vector<double> peak_values;
	std::vector<double> variation_values;
	family_values.reserve(records.size());
	block_order_values.reserve(records.size());
	order_values.reserve(records.size());
	within_block_values.reserve(records.size());
	mean_values.reserve(records.size());
	peak_values.reserve(records.size());
	variation_values.reserve(records.size());

	for (std::size_t i = 0; i < records.size(); ++i) {
		std::ostringstream block;
		block << "block ";
		if (records[i].family_index < 10) {
			block << '0';
		}
		block << records[i].family_index << " / " << records[i].family_label;
		family_values.push_back(records[i].family_label);
		block_order_values.push_back(block.str());
		order_values.push_back(static_cast<double>(i));
		within_block_values.push_back(static_cast<double>(records[i].variant_index));
		mean_values.push_back(curve_mean(records[i].curve));
		peak_values.push_back(curve_peak(records[i].curve));
		variation_values.push_back(curve_total_variation(records[i].curve));
	}

	std::vector<std::string> properties;
	properties.push_back(categorical_property_json(
		"process-family", "process family", records, family_values,
		visual::object({visual::string_field("source", "process curve native fixture family")})));
	properties.push_back(categorical_property_json(
		"matrix-block-order", "matrix block / order", records, block_order_values,
		visual::object({visual::string_field("purpose", "categorical matrix block labels in exported record order"),
						visual::field("block_ranges", block_ranges_json())})));
	properties.push_back(scalar_property_json(
		"matrix-order-index", "matrix order index", records, order_values,
		visual::object({visual::string_field("purpose", "row and column order used by the relation matrix")})));
	properties.push_back(scalar_property_json("within-block-order", "within-block order", records, within_block_values));
	properties.push_back(scalar_property_json("curve-mean", "curve mean", records, mean_values));
	properties.push_back(scalar_property_json("curve-peak", "curve peak", records, peak_values));
	properties.push_back(scalar_property_json("curve-total-variation", "curve total variation", records,
											  variation_values));
	return visual::array_of(properties);
}

auto nearest_neighbors(const Matrix &distances, std::size_t row, std::size_t k) -> std::vector<Neighbor>
{
	std::vector<Neighbor> candidates;
	candidates.reserve(distances.size() - 1);
	for (std::size_t column = 0; column < distances.size(); ++column) {
		if (row == column) {
			continue;
		}
		candidates.push_back({column, distances[row][column]});
	}
	std::sort(candidates.begin(), candidates.end(), [](const Neighbor &lhs, const Neighbor &rhs) {
		if (lhs.distance != rhs.distance) {
			return lhs.distance < rhs.distance;
		}
		return lhs.index < rhs.index;
	});
	if (candidates.size() > k) {
		candidates.resize(k);
	}
	return candidates;
}

auto graph_edges(const std::vector<Record> &records, const Matrix &distances) -> std::vector<visual::GraphEdge>
{
	std::vector<visual::GraphEdge> edges;
	edges.reserve(records.size() * kNearestNeighbors);
	for (std::size_t row = 0; row < records.size(); ++row) {
		for (const auto &neighbor : nearest_neighbors(distances, row, kNearestNeighbors)) {
			edges.push_back({records[row].id, records[neighbor.index].id, neighbor.distance});
		}
	}
	return edges;
}

auto graph_json(const std::vector<Record> &records, const Matrix &distances) -> std::string
{
	std::vector<std::string> edges;
	edges.reserve(records.size() * kNearestNeighbors);
	for (const auto &edge : graph_edges(records, distances)) {
		edges.push_back(visual::object({visual::string_field("source", edge.source_id),
										visual::string_field("target", edge.target_id),
										visual::number_field("value", edge.weight),
										visual::number_field("weight", edge.weight)}));
	}
	return visual::array_of({visual::object({
		visual::string_field("id", kGraphId),
		visual::string_field("dataset_id", kDatasetId),
		visual::string_array_field("node_record_ids", record_ids(records)),
		visual::string_field("edge_relation_id", kRelationId),
		visual::string_field("graph_type", "k-nearest graph"),
		visual::field("edges", visual::array_of(edges)),
		visual::field("metadata", visual::object({visual::size_field("neighbors_per_record", kNearestNeighbors),
												  visual::bool_field("directed", true),
												  visual::string_field("edge_source",
																	   "native aligned process-curve metric")})),
	})});
}

auto coordinates(const std::vector<Record> &records) -> std::vector<visual::Position>
{
	const std::size_t family_count = family_templates().size();
	std::vector<visual::Position> positions;
	positions.reserve(records.size());
	for (const auto &record : records) {
		const double family_unit = family_count <= 1 ? 0.0
													 : static_cast<double>(record.family_index) /
														   static_cast<double>(family_count - 1);
		const double order_unit = kVariantsPerFamily <= 1
									  ? 0.0
									  : static_cast<double>(record.variant_index) /
											static_cast<double>(kVariantsPerFamily - 1);
		const double angle = (family_unit * 2.0 * kPi) - kPi * 0.5;
		const double radius = 1.2 + 0.18 * std::sin(static_cast<double>(record.variant_index + 1) * 0.37);
		std::vector<double> position = {
			radius * std::cos(angle) + (order_unit - 0.5) * 0.65,
			0.55 * static_cast<double>(record.family_index),
			radius * std::sin(angle) + 0.12 * curve_mean(record.curve),
		};
		positions.push_back({record.id, position});
	}
	return positions;
}

auto coordinates_json(const std::vector<Record> &records) -> std::string
{
	std::vector<std::string> positions;
	for (const auto &position : coordinates(records)) {
		positions.push_back(visual::object({visual::string_field("record_id", position.record_id),
											visual::number_array_field("position", position.position)}));
	}
	return visual::array_of({visual::object({
		visual::string_field("id", kCoordinateId),
		visual::string_field("dataset_id", kDatasetId),
		visual::string_field("space_id", kSpaceId),
		visual::string_field("name", "deterministic block visualization layout"),
		visual::size_field("dimension", 3),
		visual::field("record_positions", visual::array_of(positions)),
		visual::field("metadata",
					  visual::object({
						  visual::string_field("layout_source", "deterministic C++ family/order layout"),
						  visual::bool_field("visualization_layout", true),
						  visual::bool_field("metric_embedding", false),
						  visual::string_field(
							  "note",
							  "Coordinates are for graph and block inspection only; metric values come from the exported relation."),
					  })),
	})});
}

auto diagnostic_payload_json(const std::vector<Record> &records, const MetricLawDiagnostics &law) -> std::string
{
	return visual::object({
		visual::size_field("record_count", records.size()),
		visual::size_field("family_count", family_templates().size()),
		visual::size_field("variants_per_family", kVariantsPerFamily),
		visual::size_field("relation_pair_count", law.pair_count),
		visual::size_field("knn_edge_count", records.size() * kNearestNeighbors),
		visual::string_field("native_source", "examples/engine/process_curves_space.cpp"),
		visual::string_field("metric_operator", "AlignedCurveDistance"),
		visual::field("metric_law", metric_law_json(law)),
		visual::string_field("record_order", "records are emitted contiguously by process family block"),
	});
}

auto diagnostics_json(const std::vector<Record> &records, const MetricLawDiagnostics &law) -> std::string
{
	return visual::array_of({visual::object({visual::string_field("id", "relation-matrix-native-summary"),
											 visual::string_field("dataset_id", kDatasetId),
											 visual::string_field("diagnostic_type", "native-export-summary"),
											 visual::field("payload", diagnostic_payload_json(records, law))})});
}

auto views_json() -> std::string
{
	return visual::array_of({
		visual::object({visual::string_field("id", "process-curve-relation-matrix-view"),
						visual::string_field("kind", "relation-matrix"),
						visual::string_field("name", "Aligned process-curve distance matrix"),
						visual::string_field("relationId", kRelationId)}),
		visual::object({visual::string_field("id", "process-curve-neighborhood-view"),
						visual::string_field("kind", "neighborhood-graph"),
						visual::string_field("name", "Process-curve k-nearest graph"),
						visual::string_field("spaceId", kSpaceId),
						visual::string_field("relationId", kRelationId),
						visual::string_field("graphId", kGraphId),
						visual::string_field("coordinateId", kCoordinateId),
						visual::string_field("colorPropertyId", "matrix-block-order")}),
	});
}

auto build_visual_document() -> std::string
{
	const auto records = make_records();
	const auto distances = distance_matrix(records);
	const auto law = diagnose_metric_law(distances);
	const auto ids = record_ids(records);

	return visual::object({
		visual::string_field("schema", "metric.visual.v1"),
		visual::field("provenance", visual::object({visual::string_field("writer", "examples/engine/relation_matrix_visual_export.cpp"),
													visual::string_field("writer_language", "C++17"),
													visual::string_field("computation", "native C++"),
													visual::bool_field("native_export", true),
													visual::bool_field("synthetic", false),
													visual::string_field("source_example", "examples/engine/process_curves_space.cpp"),
													visual::string_field("status", "relation_matrix_export_foundation")})),
		visual::field("datasets", visual::array_of({visual::object({
									  visual::string_field("id", kDatasetId),
									  visual::string_field("title", "Process Curve Relation Matrix"),
									  visual::string_field(
										  "description",
										  "Native C++ process-curve gallery ordered by family block for relation matrix and nearest-neighbor graph rendering."),
									  visual::string_field(
										  "source",
										  "examples/engine/process_curves_space.cpp deterministic native fixture expansion"),
									  visual::string_field("license", "MPL-2.0"),
								  })})),
		visual::field("records", records_json(records)),
		visual::field("relations", visual::array_of({relation_json(records, distances, law)})),
		visual::field("spaces", visual::array_of({visual::object({
									  visual::string_field("id", kSpaceId),
									  visual::string_field("dataset_id", kDatasetId),
									  visual::string_array_field("record_ids", ids),
									  visual::string_field("primary_relation_id", kRelationId),
									  visual::string_field("space_type", "finite_metric_space"),
									  visual::field("metadata",
													visual::object({visual::size_field("record_count", records.size()),
																	visual::string_field("metric_operator", "AlignedCurveDistance"),
																	visual::string_field("record_order", "family block order"),
																	visual::field("block_ranges", block_ranges_json())})),
								  })})),
		visual::field("properties", properties_json(records)),
		visual::field("graphs", graph_json(records, distances)),
		visual::field("coordinates", coordinates_json(records)),
		visual::field("timelines", "[]"),
		visual::field("events", "[]"),
		visual::field("views", views_json()),
		visual::field("diagnostics", diagnostics_json(records, law)),
	});
}

auto usage(const char *program) -> void
{
	std::cerr << "Usage: " << program << " [--export-dir <dir>]\n"
			  << "  Without --export-dir, writes metric.visual.v1 JSON to stdout.\n"
			  << "  With --export-dir, writes <dir>/metric.visual.json.\n";
}

} // namespace

int main(int argc, char **argv)
{
	try {
		std::string export_dir;
		for (int index = 1; index < argc; ++index) {
			const std::string arg = argv[index];
			if (arg == "--export-dir") {
				if (index + 1 >= argc) {
					usage(argv[0]);
					return 2;
				}
				export_dir = argv[++index];
			} else if (arg == "--help" || arg == "-h") {
				usage(argv[0]);
				return 0;
			} else {
				usage(argv[0]);
				return 2;
			}
		}

		const std::string document = build_visual_document();
		if (export_dir.empty()) {
			std::cout << document << '\n';
			return 0;
		}

		const std::filesystem::path directory(export_dir);
		const std::filesystem::path output_path = directory / "metric.visual.json";
		if (!visual::write_metric_visual_file(directory, document + "\n")) {
			throw std::runtime_error("failed to write export output: " + output_path.string());
		}
		return 0;
	} catch (const std::exception &error) {
		std::cerr << "relation_matrix_visual_export: " << error.what() << '\n';
		return 1;
	}
}
