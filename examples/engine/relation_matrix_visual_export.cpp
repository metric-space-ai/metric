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
#include <fstream>
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

auto json_object(const std::vector<std::pair<std::string, std::string>> &fields) -> std::string
{
	std::string out = "{";
	for (std::size_t i = 0; i < fields.size(); ++i) {
		if (i != 0) {
			out += ",";
		}
		out += visual::quote(fields[i].first);
		out += ":";
		out += fields[i].second;
	}
	out += "}";
	return out;
}

auto json_bool(bool value) -> std::string { return value ? "true" : "false"; }

auto json_size(std::size_t value) -> std::string { return std::to_string(value); }

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
		ranges.push_back(json_object({{"family", visual::quote(families[family_index].id)},
									  {"block_label", visual::quote(families[family_index].label)},
									  {"start", json_size(start)},
									  {"end_exclusive", json_size(start + kVariantsPerFamily)}}));
	}
	return visual::array_of(ranges);
}

auto records_json(const std::vector<Record> &records) -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(records.size());
	for (const auto &record : records) {
		const std::string payload = json_object({
			{"kind", visual::quote("time_series")},
			{"series", visual::number_array(record.curve)},
			{"sample_rate_hz", "1"},
			{"family", visual::quote(record.family_id)},
			{"family_label", visual::quote(record.family_label)},
			{"variant_index", json_size(record.variant_index)},
			{"native_metric_source", visual::quote("examples/engine/process_curves_space.cpp:AlignedCurveDistance")},
		});
		entries.push_back(json_object({{"id", visual::quote(record.id)},
									   {"dataset_id", visual::quote(kDatasetId)},
									   {"record_type", visual::quote("process_curve")},
									   {"label", visual::quote(record.label)},
									   {"payload", payload}}));
	}
	return visual::array_of(entries);
}

auto relation_values_json(const std::vector<Record> &records, const Matrix &distances) -> std::string
{
	std::vector<std::string> values;
	values.reserve((records.size() * (records.size() - 1)) / 2);
	for (std::size_t row = 0; row < records.size(); ++row) {
		for (std::size_t column = row + 1; column < records.size(); ++column) {
			values.push_back(json_object({{"row_id", visual::quote(records[row].id)},
										  {"column_id", visual::quote(records[column].id)},
										  {"value", visual::num(distances[row][column])}}));
		}
	}
	return visual::array_of(values);
}

auto metric_law_json(const MetricLawDiagnostics &law) -> std::string
{
	const bool diagonal_ok = law.diagonal_max_abs <= 1e-10;
	const bool symmetry_ok = law.symmetry_max_abs <= 1e-10;
	const bool triangle_ok = law.triangle_max_violation <= 1e-9;
	return json_object({{"checked", visual::quote("full finite native C++ distance matrix")},
						{"operator", visual::quote("AlignedCurveDistance")},
						{"finite", json_bool(law.finite)},
						{"diagonal_zero", json_bool(diagonal_ok)},
						{"symmetric", json_bool(symmetry_ok)},
						{"triangle", json_bool(triangle_ok)},
						{"diagonal_max_abs", visual::num(law.diagonal_max_abs)},
						{"symmetry_max_abs", visual::num(law.symmetry_max_abs)},
						{"triangle_max_violation", visual::num(law.triangle_max_violation)},
						{"min_nonzero_distance", visual::num(law.min_nonzero_distance)},
						{"max_distance", visual::num(law.max_distance)},
						{"mean_distance", visual::num(law.mean_distance)},
						{"pair_count", json_size(law.pair_count)},
						{"triangle_triplets", json_size(law.triangle_triplets)}});
}

auto relation_json(const std::vector<Record> &records, const Matrix &distances, const MetricLawDiagnostics &law)
	-> std::string
{
	const auto ids = record_ids(records);
	const std::string metadata = json_object({
		{"symmetric", "true"},
		{"complete_upper_triangle", "true"},
		{"record_order", visual::quote("contiguous categorical family blocks")},
		{"storage_note", visual::quote("upper-triangle pair entries for symmetric dense matrix rendering")},
		{"block_ranges", block_ranges_json()},
		{"law_check", metric_law_json(law)},
	});
	return json_object({{"id", visual::quote(kRelationId)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"name", visual::quote("aligned process-curve metric")},
						{"relation_type", visual::quote("metric")},
						{"value_type", visual::quote("scalar")},
						{"record_ids", visual::string_array(ids)},
						{"storage", visual::quote("symmetric_dense_matrix")},
						{"values", relation_values_json(records, distances)},
						{"metadata", metadata}});
}

auto categorical_property_json(const std::string &id, const std::string &name, const std::vector<Record> &records,
							   const std::vector<std::string> &values, const std::string &metadata = "{}")
	-> std::string
{
	std::vector<std::string> entries;
	entries.reserve(records.size());
	for (std::size_t i = 0; i < records.size(); ++i) {
		entries.push_back(json_object({{"record_id", visual::quote(records[i].id)},
									   {"value", visual::quote(values[i])}}));
	}
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"name", visual::quote(name)},
						{"target_type", visual::quote("record")},
						{"value_type", visual::quote("categorical")},
						{"values", visual::array_of(entries)},
						{"metadata", metadata}});
}

auto scalar_property_json(const std::string &id, const std::string &name, const std::vector<Record> &records,
						  const std::vector<double> &values, const std::string &metadata = "{}") -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(records.size());
	for (std::size_t i = 0; i < records.size(); ++i) {
		entries.push_back(json_object({{"record_id", visual::quote(records[i].id)},
									   {"value", visual::num(values[i])}}));
	}
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"name", visual::quote(name)},
						{"target_type", visual::quote("record")},
						{"value_type", visual::quote("scalar")},
						{"values", visual::array_of(entries)},
						{"metadata", metadata}});
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
		json_object({{"source", visual::quote("process curve native fixture family")}})));
	properties.push_back(categorical_property_json(
		"matrix-block-order", "matrix block / order", records, block_order_values,
		json_object({{"purpose", visual::quote("categorical matrix block labels in exported record order")},
					 {"block_ranges", block_ranges_json()}})));
	properties.push_back(scalar_property_json(
		"matrix-order-index", "matrix order index", records, order_values,
		json_object({{"purpose", visual::quote("row and column order used by the relation matrix")}})));
	properties.push_back(scalar_property_json("within-block-order", "within-block order", records,
											  within_block_values));
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

auto graph_json(const std::vector<Record> &records, const Matrix &distances) -> std::string
{
	const auto ids = record_ids(records);
	std::vector<std::string> edges;
	edges.reserve(records.size() * kNearestNeighbors);
	for (std::size_t row = 0; row < records.size(); ++row) {
		for (const auto &neighbor : nearest_neighbors(distances, row, kNearestNeighbors)) {
			edges.push_back(json_object({{"source", visual::quote(records[row].id)},
										 {"target", visual::quote(records[neighbor.index].id)},
										 {"value", visual::num(neighbor.distance)},
										 {"weight", visual::num(neighbor.distance)}}));
		}
	}
	const std::string graph = json_object({
		{"id", visual::quote(kGraphId)},
		{"dataset_id", visual::quote(kDatasetId)},
		{"node_record_ids", visual::string_array(ids)},
		{"edge_relation_id", visual::quote(kRelationId)},
		{"graph_type", visual::quote("k-nearest graph")},
		{"edges", visual::array_of(edges)},
		{"metadata", json_object({{"neighbors_per_record", json_size(kNearestNeighbors)},
								   {"directed", "true"},
								   {"edge_source", visual::quote("native aligned process-curve metric")}})},
	});
	return visual::array_of({graph});
}

auto coordinates_json(const std::vector<Record> &records) -> std::string
{
	const std::size_t family_count = family_templates().size();
	std::vector<std::string> positions;
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
		positions.push_back(json_object({{"record_id", visual::quote(record.id)},
										 {"position", visual::number_array(position)}}));
	}

	const std::string coordinate = json_object({
		{"id", visual::quote(kCoordinateId)},
		{"dataset_id", visual::quote(kDatasetId)},
		{"space_id", visual::quote(kSpaceId)},
		{"name", visual::quote("deterministic block visualization layout")},
		{"dimension", "3"},
		{"record_positions", visual::array_of(positions)},
		{"metadata", json_object({{"layout_source", visual::quote("deterministic C++ family/order layout")},
								   {"visualization_layout", "true"},
								   {"metric_embedding", "false"},
								   {"note", visual::quote("Coordinates are for graph and block inspection only; metric values come from the exported relation.")}})},
	});
	return visual::array_of({coordinate});
}

auto diagnostics_json(const std::vector<Record> &records, const MetricLawDiagnostics &law) -> std::string
{
	const std::string payload = json_object({
		{"record_count", json_size(records.size())},
		{"family_count", json_size(family_templates().size())},
		{"variants_per_family", json_size(kVariantsPerFamily)},
		{"relation_pair_count", json_size(law.pair_count)},
		{"knn_edge_count", json_size(records.size() * kNearestNeighbors)},
		{"native_source", visual::quote("examples/engine/process_curves_space.cpp")},
		{"metric_operator", visual::quote("AlignedCurveDistance")},
		{"metric_law", metric_law_json(law)},
		{"record_order", visual::quote("records are emitted contiguously by process family block")},
	});
	const std::string diagnostic = json_object({{"id", visual::quote("relation-matrix-native-summary")},
											   {"dataset_id", visual::quote(kDatasetId)},
											   {"diagnostic_type", visual::quote("native-export-summary")},
											   {"payload", payload}});
	return visual::array_of({diagnostic});
}

auto views_json() -> std::string
{
	return visual::array_of({
		json_object({{"id", visual::quote("process-curve-relation-matrix-view")},
					 {"kind", visual::quote("relation-matrix")},
					 {"name", visual::quote("Aligned process-curve distance matrix")},
					 {"relationId", visual::quote(kRelationId)}}),
		json_object({{"id", visual::quote("process-curve-neighborhood-view")},
					 {"kind", visual::quote("neighborhood-graph")},
					 {"name", visual::quote("Process-curve k-nearest graph")},
					 {"spaceId", visual::quote(kSpaceId)},
					 {"relationId", visual::quote(kRelationId)},
					 {"graphId", visual::quote(kGraphId)},
					 {"coordinateId", visual::quote(kCoordinateId)},
					 {"colorPropertyId", visual::quote("matrix-block-order")}}),
	});
}

auto build_visual_document() -> std::string
{
	const auto records = make_records();
	const auto distances = distance_matrix(records);
	const auto law = diagnose_metric_law(distances);
	const auto ids = record_ids(records);

	return json_object({
		{"schema", visual::quote("metric.visual.v1")},
		{"provenance", json_object({{"writer", visual::quote("examples/engine/relation_matrix_visual_export.cpp")},
									{"writer_language", visual::quote("C++17")},
									{"computation", visual::quote("native C++")},
									{"native_export", "true"},
									{"synthetic", "false"},
									{"source_example", visual::quote("examples/engine/process_curves_space.cpp")},
									{"status", visual::quote("relation_matrix_export_foundation")}})},
		{"datasets", visual::array_of({json_object({{"id", visual::quote(kDatasetId)},
													 {"title", visual::quote("Process Curve Relation Matrix")},
													 {"description", visual::quote("Native C++ process-curve gallery ordered by family block for relation matrix and nearest-neighbor graph rendering.")},
													 {"source", visual::quote("examples/engine/process_curves_space.cpp deterministic native fixture expansion")},
													 {"license", visual::quote("MPL-2.0")}})})},
		{"records", records_json(records)},
		{"relations", visual::array_of({relation_json(records, distances, law)})},
		{"spaces", visual::array_of({json_object({{"id", visual::quote(kSpaceId)},
												   {"dataset_id", visual::quote(kDatasetId)},
												   {"record_ids", visual::string_array(ids)},
												   {"primary_relation_id", visual::quote(kRelationId)},
												   {"space_type", visual::quote("finite_metric_space")},
												   {"metadata", json_object({{"record_count", json_size(records.size())},
																			 {"metric_operator", visual::quote("AlignedCurveDistance")},
																			 {"record_order", visual::quote("family block order")},
																			 {"block_ranges", block_ranges_json()}})}})})},
		{"properties", properties_json(records)},
		{"graphs", graph_json(records, distances)},
		{"coordinates", coordinates_json(records)},
		{"timelines", "[]"},
		{"events", "[]"},
		{"views", views_json()},
		{"diagnostics", diagnostics_json(records, law)},
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

		const std::string json = build_visual_document();
		if (export_dir.empty()) {
			std::cout << json << '\n';
			return 0;
		}

		const std::filesystem::path directory(export_dir);
		std::filesystem::create_directories(directory);
		const std::filesystem::path output_path = directory / "metric.visual.json";
		std::ofstream out(output_path);
		if (!out) {
			throw std::runtime_error("failed to open export output: " + output_path.string());
		}
		out << json << '\n';
		return 0;
	} catch (const std::exception &error) {
		std::cerr << "relation_matrix_visual_export: " << error.what() << '\n';
		return 1;
	}
}
