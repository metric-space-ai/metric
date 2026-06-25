// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Native metric.visual.v1 exporter for mapping and dimensionality evidence.
//
// The evidence is computed by a native C++ process-curve diffusion-coordinate
// pipeline: records are deterministic process curves, distances are the
// aligned-curve metric, diffusion targets are built from the native distance
// matrix, and mapped coordinates/diagnostics come from a native parametric
// linear coordinate solver.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "mtrc_visual.hpp"

namespace {

namespace visual = mtrc::visual;

constexpr const char *kDatasetId = "mapping-dimensionality-process-curves";
constexpr const char *kSourceRelationId = "process-curve-aligned-metric";
constexpr const char *kLatentRelationId = "process-curve-latent-euclidean-metric";
constexpr const char *kSourceSpaceId = "process-curve-source-space";
constexpr const char *kLatentSpaceId = "process-curve-latent-space";
constexpr const char *kGraphId = "process-curve-source-knn";
constexpr const char *kMorphTimelineId = "mapping-coordinate-morph";
constexpr std::size_t kVariantsPerFamily = 200;
constexpr std::size_t kCoordinateCount = 8;
constexpr std::size_t kNeighborCount = 5;
constexpr double kEps = 1.0e-9;
constexpr double kRidgeLambda = 1.0e-4;

struct ProcessCurve {
	std::string id;
	std::string family;
	std::string family_label;
	std::size_t family_index{};
	std::size_t variant_index{};
	std::vector<double> values;
};

struct FamilyTemplate {
	std::string id;
	std::string label;
	std::vector<double> base;
};

struct AlignedCurveDistance {
	double gap_cost{2.0};

	auto operator()(const ProcessCurve &lhs, const ProcessCurve &rhs) const -> double
	{
		std::vector<double> previous(rhs.values.size() + 1, 0.0);
		for (std::size_t index = 0; index < previous.size(); ++index) {
			previous[index] = static_cast<double>(index) * gap_cost;
		}

		for (std::size_t lhs_index = 1; lhs_index <= lhs.values.size(); ++lhs_index) {
			std::vector<double> current(rhs.values.size() + 1, 0.0);
			current[0] = static_cast<double>(lhs_index) * gap_cost;
			for (std::size_t rhs_index = 1; rhs_index <= rhs.values.size(); ++rhs_index) {
				const auto substitution =
					previous[rhs_index - 1] +
					std::min(std::abs(lhs.values[lhs_index - 1] - rhs.values[rhs_index - 1]), 2 * gap_cost);
				const auto deletion = previous[rhs_index] + gap_cost;
				const auto insertion = current[rhs_index - 1] + gap_cost;
				current[rhs_index] = std::min({substitution, deletion, insertion});
			}
			previous = std::move(current);
		}

		return previous.back();
	}
};

struct CurveCoordinateEncoder {
	std::size_t coordinate_count{0};

	auto operator()(const ProcessCurve &record) const -> std::vector<double>
	{
		if (coordinate_count == 0) {
			throw std::invalid_argument("curve coordinate encoder requires a positive coordinate count");
		}
		if (record.values.empty()) {
			throw std::invalid_argument("curve coordinate encoder requires non-empty curve values");
		}
		if (coordinate_count == 1 || record.values.size() == 1) {
			return std::vector<double>(coordinate_count, record.values.front());
		}

		std::vector<double> coordinates;
		coordinates.reserve(coordinate_count);
		for (std::size_t index = 0; index < coordinate_count; ++index) {
			const auto position = static_cast<double>(index) * static_cast<double>(record.values.size() - 1) /
								  static_cast<double>(coordinate_count - 1);
			const auto left = static_cast<std::size_t>(std::floor(position));
			const auto right = std::min(left + 1, record.values.size() - 1);
			const auto fraction = position - static_cast<double>(left);
			coordinates.push_back(record.values[left] * (1.0 - fraction) + record.values[right] * fraction);
		}
		return coordinates;
	}
};

struct MetricLawDiagnostics {
	double min_distance{std::numeric_limits<double>::infinity()};
	double max_distance{};
	double mean_distance{};
	double diagonal_max_abs{};
	double symmetry_max_abs{};
	double triangle_max_violation{};
	std::size_t pair_count{};
	std::size_t triangle_triplets{};
	bool triangle_complete{false};
	bool finite{true};
};

auto json_object(const std::vector<std::pair<std::string, std::string>> &fields) -> std::string
{
	std::vector<visual::Field> visual_fields;
	visual_fields.reserve(fields.size());
	for (const auto &field : fields) {
		visual_fields.push_back(visual::field(field.first, field.second));
	}
	return visual::object(visual_fields);
}

auto json_bool(bool value) -> std::string { return visual::boolean(value); }

auto json_size(std::size_t value) -> std::string { return visual::size(value); }

auto json_number_array(const std::vector<double> &values) -> std::string
{
	return visual::number_array(values);
}

auto record_id(std::size_t index) -> std::string
{
	std::ostringstream out;
	out << "pc-";
	if (index < 1000) {
		out << '0';
	}
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
	return {{"normal_reference", "normal reference", {0, 0, 1, 1, 1, 2, 3, 3}},
			{"flat_hold", "flat hold", {0, 0, 0, 0, 1, 1, 1, 1, 1, 1}},
			{"late_ramp", "late ramp", {0, 0, 0, 1, 1, 1, 2, 3, 3}},
			{"spike", "spike", {0, 0, 1, 7, 1, 2, 3, 3}},
			{"early_ramp", "early ramp", {0, 1, 1, 1, 2, 3, 3, 3}}};
}

auto variant_curve(const FamilyTemplate &family, std::size_t family_index, std::size_t variant_index)
	-> std::vector<double>
{
	std::vector<double> curve;
	curve.reserve(family.base.size() + 3);

	const double centered = static_cast<double>(variant_index) -
							(static_cast<double>(kVariantsPerFamily) - 1.0) * 0.5;
	const double unit = kVariantsPerFamily <= 1
							? 0.0
							: static_cast<double>(variant_index) / static_cast<double>(kVariantsPerFamily - 1);
	const double scale = 0.88 + 0.24 * unit + 0.012 * std::sin(0.091 * static_cast<double>(variant_index + 1));
	const double shift = 0.012 * static_cast<double>((variant_index % 13) + 1);
	const double slope = 0.08 * (unit - 0.5) + 0.006 * static_cast<double>(family_index);

	for (std::size_t sample = 0; sample < family.base.size(); ++sample) {
		const double t = family.base.size() <= 1
							 ? 0.0
							 : static_cast<double>(sample) / static_cast<double>(family.base.size() - 1);
		const double ripple =
			0.05 * std::sin((static_cast<double>(family_index) + 1.0) *
							 (static_cast<double>(variant_index) + 3.0) *
							 (static_cast<double>(sample) + 1.0) * 0.137);
		double value = family.base[sample] * scale + shift + slope * t + ripple;
		if (family.id == "flat_hold" && sample >= 4) {
			value += 0.02 * static_cast<double>(variant_index % 9);
		}
		if (family.id == "late_ramp" && sample < 4) {
			value -= 0.015 * static_cast<double>(variant_index % 11);
		}
		if (family.id == "early_ramp" && sample <= 2) {
			value += 0.018 * static_cast<double>(variant_index % 10);
		}
		if (family.id == "spike" && (sample == 3 || sample == 4)) {
			value += 0.11 * static_cast<double>(variant_index % 17) - 0.75;
		}
		curve.push_back(std::max(0.0, value));
	}

	if (variant_index % 5 == 1) {
		curve.insert(curve.begin(), curve.front());
	} else if (variant_index % 5 == 2) {
		curve.push_back(curve.back());
	} else if (variant_index % 5 == 3 && curve.size() > 7) {
		const auto insert_at = curve.begin() + static_cast<std::ptrdiff_t>(curve.size() / 2);
		curve.insert(insert_at, curve[curve.size() / 2]);
	} else if (variant_index % 5 == 4 && curve.size() > 8) {
		curve.insert(curve.begin() + 1, curve[1]);
		curve.push_back(curve.back() + 0.002 * centered);
	}

	curve.back() += 0.0008 * static_cast<double>(variant_index + 1);
	return curve;
}

auto process_records() -> std::vector<ProcessCurve>
{
	const auto families = family_templates();
	std::vector<ProcessCurve> records;
	records.reserve(families.size() * kVariantsPerFamily);
	for (std::size_t family_index = 0; family_index < families.size(); ++family_index) {
		for (std::size_t variant = 0; variant < kVariantsPerFamily; ++variant) {
			const std::size_t index = records.size();
			records.push_back({record_id(index),
							   families[family_index].id,
							   families[family_index].label,
							   family_index,
							   variant,
							   variant_curve(families[family_index], family_index, variant)});
		}
	}
	return records;
}

auto held_out_queries() -> std::vector<ProcessCurve>
{
	const auto families = family_templates();
	std::vector<ProcessCurve> queries;
	queries.reserve(families.size() * 2);
	for (std::size_t family_index = 0; family_index < families.size(); ++family_index) {
		for (std::size_t offset = 0; offset < 2; ++offset) {
			const std::size_t variant = kVariantsPerFamily + 7 * family_index + 11 * offset;
			std::ostringstream id;
			id << "gallery-query-" << families[family_index].id << '-' << offset;
			queries.push_back({id.str(),
							   families[family_index].id,
							   families[family_index].label,
							   family_index,
							   variant,
							   variant_curve(families[family_index], family_index, variant)});
		}
	}
	return queries;
}

auto record_ids(const std::vector<ProcessCurve> &records) -> std::vector<std::string>
{
	std::vector<std::string> ids;
	ids.reserve(records.size());
	for (const auto &record : records) {
		ids.push_back(record.id);
	}
	return ids;
}

auto aligned_distance_matrix(const std::vector<ProcessCurve> &records) -> std::vector<std::vector<double>>
{
	AlignedCurveDistance metric;
	std::vector<std::vector<double>> distances(records.size(), std::vector<double>(records.size(), 0.0));
	for (std::size_t row = 0; row < records.size(); ++row) {
		for (std::size_t column = row + 1; column < records.size(); ++column) {
			const auto value = metric(records[row], records[column]);
			distances[row][column] = value;
			distances[column][row] = value;
		}
	}
	return distances;
}

auto euclidean_distance_matrix(const std::vector<std::vector<double>> &positions) -> std::vector<std::vector<double>>
{
	std::vector<std::vector<double>> distances(positions.size(), std::vector<double>(positions.size(), 0.0));
	for (std::size_t row = 0; row < positions.size(); ++row) {
		for (std::size_t column = row + 1; column < positions.size(); ++column) {
			double squared = 0.0;
			const auto dimensions = std::min(positions[row].size(), positions[column].size());
			for (std::size_t axis = 0; axis < dimensions; ++axis) {
				const auto delta = positions[row][axis] - positions[column][axis];
				squared += delta * delta;
			}
			const auto value = std::sqrt(squared);
			distances[row][column] = value;
			distances[column][row] = value;
		}
	}
	return distances;
}

auto diagnose_metric_law(const std::vector<std::vector<double>> &distances) -> MetricLawDiagnostics
{
	MetricLawDiagnostics diagnostics;
	const auto size = distances.size();
	for (std::size_t row = 0; row < size; ++row) {
		diagnostics.diagonal_max_abs = std::max(diagnostics.diagonal_max_abs, std::abs(distances[row][row]));
		for (std::size_t column = 0; column < size; ++column) {
			const auto value = distances[row][column];
			diagnostics.finite = diagnostics.finite && std::isfinite(value);
			diagnostics.min_distance = std::min(diagnostics.min_distance, value);
			diagnostics.max_distance = std::max(diagnostics.max_distance, value);
			diagnostics.symmetry_max_abs =
				std::max(diagnostics.symmetry_max_abs, std::abs(value - distances[column][row]));
			if (row != column) {
				diagnostics.mean_distance += value;
				++diagnostics.pair_count;
			}
		}
	}
	const std::size_t max_triangle_triplets = 250000;
	if (size <= 64) {
		for (std::size_t row = 0; row < size; ++row) {
			for (std::size_t column = 0; column < size; ++column) {
				for (std::size_t via = 0; via < size; ++via) {
					const auto violation = distances[row][column] - distances[row][via] - distances[via][column];
					diagnostics.triangle_max_violation = std::max(diagnostics.triangle_max_violation, violation);
					++diagnostics.triangle_triplets;
				}
			}
		}
		diagnostics.triangle_complete = true;
	} else {
		const auto stride_a = std::max<std::size_t>(1, size / 53);
		const auto stride_b = std::max<std::size_t>(1, size / 47);
		const auto stride_c = std::max<std::size_t>(1, size / 43);
		for (std::size_t row = 0; row < size && diagnostics.triangle_triplets < max_triangle_triplets;
			 row += stride_a) {
			for (std::size_t column = (row * 17 + 3) % size;
				 diagnostics.triangle_triplets < max_triangle_triplets && column < size; column += stride_b) {
				for (std::size_t via = (row + column * 11 + 5) % size;
					 diagnostics.triangle_triplets < max_triangle_triplets && via < size; via += stride_c) {
					const auto violation = distances[row][column] - distances[row][via] - distances[via][column];
					diagnostics.triangle_max_violation = std::max(diagnostics.triangle_max_violation, violation);
					++diagnostics.triangle_triplets;
				}
			}
		}
		diagnostics.triangle_complete = false;
	}
	if (diagnostics.pair_count != 0) {
		diagnostics.mean_distance /= static_cast<double>(diagnostics.pair_count);
	}
	if (!std::isfinite(diagnostics.min_distance)) {
		diagnostics.min_distance = 0.0;
	}
	return diagnostics;
}

auto metric_law_json(const MetricLawDiagnostics &law, const std::string &operator_name) -> std::string
{
	return json_object({{"checked", visual::quote(law.triangle_complete
													  ? "full finite native C++ distance matrix"
													  : "native C++ dense matrix with deterministic triangle audit")},
						{"operator", visual::quote(operator_name)},
						{"finite", json_bool(law.finite)},
						{"non_negative", json_bool(law.min_distance >= -kEps)},
						{"diagonal_zero", json_bool(law.diagonal_max_abs <= kEps)},
						{"symmetric", json_bool(law.symmetry_max_abs <= kEps)},
						{"triangle", json_bool(law.triangle_max_violation <= kEps)},
						{"triangle_complete", json_bool(law.triangle_complete)},
						{"min_distance", visual::num(law.min_distance)},
						{"max_distance", visual::num(law.max_distance)},
						{"mean_distance", visual::num(law.mean_distance)},
						{"diagonal_max_abs", visual::num(law.diagonal_max_abs)},
						{"symmetry_max_abs", visual::num(law.symmetry_max_abs)},
						{"triangle_max_violation", visual::num(law.triangle_max_violation)},
						{"pair_count", json_size(law.pair_count)},
						{"triangle_triplets", json_size(law.triangle_triplets)}});
}

auto relation_values_json(const std::vector<std::vector<double>> &distances) -> std::string
{
	std::vector<std::string> rows;
	rows.reserve(distances.size());
	for (const auto &row : distances) {
		rows.push_back(visual::number_array(row));
	}
	return visual::array_of(rows);
}

auto relation_json(const std::string &id, const std::string &name, const std::string &relation_type,
				   const std::vector<std::string> &ids, const std::vector<std::vector<double>> &distances,
				   const std::string &metadata) -> std::string
{
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"name", visual::quote(name)},
						{"relation_type", visual::quote(relation_type)},
						{"value_type", visual::quote("scalar")},
						{"record_ids", visual::string_array(ids)},
						{"storage", visual::quote("symmetric_dense_matrix")},
						{"values", relation_values_json(distances)},
						{"metadata", metadata}});
}

auto space_json(const std::string &id, const std::string &primary_relation_id, const std::string &space_type,
				const std::vector<std::string> &ids, const std::string &metadata) -> std::string
{
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"record_ids", visual::string_array(ids)},
						{"primary_relation_id", visual::quote(primary_relation_id)},
						{"space_type", visual::quote(space_type)},
						{"metadata", metadata}});
}

auto categorical_property_json(const std::string &id, const std::string &name, const std::vector<std::string> &ids,
							   const std::vector<std::string> &values, const std::string &metadata = "{}")
	-> std::string
{
	std::vector<std::string> entries;
	entries.reserve(ids.size());
	for (std::size_t index = 0; index < ids.size(); ++index) {
		entries.push_back(json_object({{"record_id", visual::quote(ids[index])},
									   {"value", visual::quote(values[index])}}));
	}
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"name", visual::quote(name)},
						{"target_type", visual::quote("record")},
						{"value_type", visual::quote("categorical")},
						{"values", visual::array_of(entries)},
						{"metadata", metadata}});
}

auto scalar_property_json(const std::string &id, const std::string &name, const std::vector<std::string> &ids,
						  const std::vector<double> &values, const std::string &metadata = "{}") -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(ids.size());
	for (std::size_t index = 0; index < ids.size(); ++index) {
		entries.push_back(json_object({{"record_id", visual::quote(ids[index])},
									   {"value", visual::num(values[index])}}));
	}
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"name", visual::quote(name)},
						{"target_type", visual::quote("record")},
						{"value_type", visual::quote("scalar")},
						{"values", visual::array_of(entries)},
						{"metadata", metadata}});
}

auto vector_norm(const std::vector<double> &values) -> double
{
	double total = 0.0;
	for (const auto value : values) {
		total += value * value;
	}
	return std::sqrt(total);
}

auto curve_mean(const ProcessCurve &record) -> double
{
	double total = 0.0;
	for (const auto value : record.values) {
		total += value;
	}
	return record.values.empty() ? 0.0 : total / static_cast<double>(record.values.size());
}

auto curve_total_variation(const ProcessCurve &record) -> double
{
	double total = 0.0;
	for (std::size_t index = 1; index < record.values.size(); ++index) {
		total += std::abs(record.values[index] - record.values[index - 1]);
	}
	return total;
}

auto source_coordinate_positions(const std::vector<ProcessCurve> &records) -> std::vector<std::vector<double>>
{
	CurveCoordinateEncoder encoder{kCoordinateCount};
	std::vector<std::vector<double>> positions;
	positions.reserve(records.size());
	for (const auto &record : records) {
		const auto coordinates = encoder(record);
		positions.push_back({coordinates.front(), coordinates[coordinates.size() / 2], coordinates.back()});
	}
	return positions;
}

auto coordinate_json(const std::string &id, const std::string &space_id, const std::string &name,
					 const std::vector<std::string> &ids, const std::vector<std::vector<double>> &positions,
					 const std::string &metadata) -> std::string
{
	if (positions.empty()) {
		throw std::invalid_argument("coordinate export requires at least one position");
	}
	const auto dimension = positions.front().size();
	std::vector<std::string> entries;
	entries.reserve(ids.size());
	for (std::size_t index = 0; index < ids.size(); ++index) {
		entries.push_back(json_object({{"record_id", visual::quote(ids[index])},
									   {"position", json_number_array(positions[index])}}));
	}
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"space_id", visual::quote(space_id)},
						{"name", visual::quote(name)},
						{"dimension", json_size(dimension)},
						{"record_positions", visual::array_of(entries)},
						{"metadata", metadata}});
}

auto nearest_indices(const std::vector<std::vector<double>> &distances, std::size_t row, std::size_t count)
	-> std::vector<std::size_t>
{
	std::vector<std::pair<double, std::size_t>> candidates;
	candidates.reserve(distances.size() - 1);
	for (std::size_t column = 0; column < distances.size(); ++column) {
		if (row == column) {
			continue;
		}
		candidates.push_back({distances[row][column], column});
	}
	std::sort(candidates.begin(), candidates.end());
	if (candidates.size() > count) {
		candidates.resize(count);
	}

	std::vector<std::size_t> out;
	out.reserve(candidates.size());
	for (const auto &candidate : candidates) {
		out.push_back(candidate.second);
	}
	return out;
}

auto knn_graph_json(const std::vector<std::string> &ids, const std::vector<std::vector<double>> &distances)
	-> std::string
{
	std::vector<std::string> edges;
	edges.reserve(ids.size() * kNeighborCount);
	for (std::size_t row = 0; row < ids.size(); ++row) {
		for (const auto column : nearest_indices(distances, row, kNeighborCount)) {
			edges.push_back(json_object({{"source_id", visual::quote(ids[row])},
										 {"target_id", visual::quote(ids[column])},
										 {"weight", visual::num(distances[row][column])}}));
		}
	}
	return json_object({{"id", visual::quote(kGraphId)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"node_record_ids", visual::string_array(ids)},
						{"edge_relation_id", visual::quote(kSourceRelationId)},
						{"graph_type", visual::quote("k-nearest")},
						{"edges", visual::array_of(edges)},
						{"metadata", json_object({{"neighbors_per_record", json_size(kNeighborCount)},
												   {"edge_source", visual::quote("native aligned curve metric")}})}});
}

auto row_mean_distance(const std::vector<std::vector<double>> &distances, std::size_t row) -> double
{
	if (distances.size() < 2) {
		return 0.0;
	}
	double total = 0.0;
	for (std::size_t column = 0; column < distances.size(); ++column) {
		if (row != column) {
			total += distances[row][column];
		}
	}
	return total / static_cast<double>(distances.size() - 1);
}

auto local_distortion(const std::vector<std::vector<double>> &source, const std::vector<std::vector<double>> &mapped)
	-> std::vector<double>
{
	const auto source_law = diagnose_metric_law(source);
	const auto mapped_law = diagnose_metric_law(mapped);
	const auto source_scale = std::max(source_law.mean_distance, kEps);
	const auto mapped_scale = std::max(mapped_law.mean_distance, kEps);
	std::vector<double> values(source.size(), 0.0);
	for (std::size_t row = 0; row < source.size(); ++row) {
		double total = 0.0;
		std::size_t count = 0;
		for (std::size_t column = 0; column < source.size(); ++column) {
			if (row == column) {
				continue;
			}
			total += std::abs((source[row][column] / source_scale) - (mapped[row][column] / mapped_scale));
			++count;
		}
		values[row] = count == 0 ? 0.0 : total / static_cast<double>(count);
	}
	return values;
}

auto target_residuals(const std::vector<std::vector<double>> &target_positions,
					  const std::vector<std::vector<double>> &latent_positions) -> std::vector<double>
{
	std::vector<double> residuals;
	residuals.reserve(target_positions.size());
	for (std::size_t row = 0; row < target_positions.size(); ++row) {
		std::vector<double> delta;
		delta.reserve(target_positions[row].size());
		for (std::size_t axis = 0; axis < target_positions[row].size(); ++axis) {
			delta.push_back(latent_positions[row][axis] - target_positions[row][axis]);
		}
		residuals.push_back(vector_norm(delta));
	}
	return residuals;
}

struct DiffusionTargets {
	std::vector<std::vector<double>> coordinates;
	std::vector<std::size_t> anchor_indices;
	double kernel_scale{};
	std::size_t dense_distance_evaluations{};
	std::string method{"single_step_diffusion_potential_anchor_coordinates"};
	std::string pairwise_distances{"native_aligned_curve_dense_distance_matrix"};
	std::string affinity_kernel{"exponential_affinity_kernel"};
	std::string diffusion_operator{"lazy_row_normalized_diffusion_operator"};
};

struct LinearCoordinateMap {
	std::vector<std::vector<double>> weights;
	std::size_t input_dimensions{};
	std::size_t output_dimensions{};
	double initial_mse{};
	double final_mse{};
	std::string mapping{"parametric_diffusion_coordinates"};
	std::string strategy{"native_ridge_linear_diffusion_coordinate_solver"};
};

struct PreservationDiagnostics {
	std::vector<double> per_record_error;
	std::vector<std::size_t> source_best;
	std::vector<std::size_t> latent_best;
	std::vector<std::size_t> source_best_latent_rank;
	std::vector<double> source_best_distance;
	std::vector<double> latent_best_distance;
	std::size_t matched_neighbors{};
	std::size_t possible_neighbors{};
	double recall{};
	double average_row_recall{};
	bool exact{true};
};

struct OutOfSampleDiagnostics {
	std::size_t query_count{};
	std::size_t evaluated_neighbor_count{};
	std::size_t anchor_matches{};
	std::size_t first_anchor_matches{};
	double anchor_recall{};
	double first_anchor_match_rate{};
	double average_mapped_best_source_rank{};
	std::size_t maximum_mapped_best_source_rank{};
	double average_best_distance_penalty{};
	double maximum_best_distance_penalty{};
	std::vector<std::size_t> source_best;
	std::vector<std::size_t> latent_best;
	std::vector<std::size_t> latent_source_rank;
	std::vector<double> source_best_distance;
	std::vector<double> latent_distance;
	std::vector<double> best_distance_penalty;
};

auto positive_mean_distance(const std::vector<std::vector<double>> &distances) -> double
{
	double total = 0.0;
	std::size_t count = 0;
	for (std::size_t row = 0; row < distances.size(); ++row) {
		for (std::size_t column = row + 1; column < distances.size(); ++column) {
			if (distances[row][column] > kEps) {
				total += distances[row][column];
				++count;
			}
		}
	}
	return count == 0 ? 1.0 : total / static_cast<double>(count);
}

auto diffusion_anchor_indices(const std::vector<std::vector<double>> &distances, std::size_t dimensions)
	-> std::vector<std::size_t>
{
	std::vector<std::size_t> anchors;
	if (distances.empty() || dimensions == 0) {
		return anchors;
	}
	anchors.push_back(0);
	while (anchors.size() < dimensions) {
		std::size_t best_index = 0;
		double best_distance = -1.0;
		for (std::size_t candidate = 0; candidate < distances.size(); ++candidate) {
			if (std::find(anchors.begin(), anchors.end(), candidate) != anchors.end()) {
				continue;
			}
			double nearest_anchor_distance = std::numeric_limits<double>::infinity();
			for (const auto anchor : anchors) {
				nearest_anchor_distance = std::min(nearest_anchor_distance, distances[candidate][anchor]);
			}
			if (nearest_anchor_distance > best_distance) {
				best_distance = nearest_anchor_distance;
				best_index = candidate;
			}
		}
		anchors.push_back(best_index);
	}
	return anchors;
}

auto center_columns_to_unit_max_abs(std::vector<std::vector<double>> &coordinates) -> void
{
	if (coordinates.empty() || coordinates.front().empty()) {
		return;
	}
	const auto dimensions = coordinates.front().size();
	for (std::size_t axis = 0; axis < dimensions; ++axis) {
		double mean = 0.0;
		for (const auto &row : coordinates) {
			mean += row[axis];
		}
		mean /= static_cast<double>(coordinates.size());
		double max_abs = 0.0;
		for (auto &row : coordinates) {
			row[axis] -= mean;
			max_abs = std::max(max_abs, std::abs(row[axis]));
		}
		if (max_abs <= kEps) {
			continue;
		}
		for (auto &row : coordinates) {
			row[axis] /= max_abs;
		}
	}
}

auto diffusion_coordinate_targets(const std::vector<std::vector<double>> &distances, std::size_t dimensions)
	-> DiffusionTargets
{
	if (distances.empty()) {
		throw std::invalid_argument("diffusion targets require a non-empty distance matrix");
	}
	DiffusionTargets targets;
	targets.anchor_indices = diffusion_anchor_indices(distances, dimensions);
	targets.kernel_scale = positive_mean_distance(distances);
	targets.dense_distance_evaluations = distances.size() * distances.size();
	targets.coordinates.assign(distances.size(), std::vector<double>(dimensions, 0.0));

	for (std::size_t row = 0; row < distances.size(); ++row) {
		double row_sum = 0.0;
		for (std::size_t column = 0; column < distances.size(); ++column) {
			row_sum += std::exp(-distances[row][column] / std::max(targets.kernel_scale, kEps));
		}
		for (std::size_t axis = 0; axis < dimensions; ++axis) {
			const auto anchor = targets.anchor_indices[axis];
			const auto affinity = std::exp(-distances[row][anchor] / std::max(targets.kernel_scale, kEps));
			double probability = row_sum <= kEps ? 0.0 : 0.5 * affinity / row_sum;
			if (row == anchor) {
				probability += 0.5;
			}
			targets.coordinates[row][axis] = -std::log(std::max(probability, kEps));
		}
	}
	center_columns_to_unit_max_abs(targets.coordinates);
	return targets;
}

auto encoded_feature_vector(const ProcessCurve &record) -> std::vector<double>
{
	CurveCoordinateEncoder encoder{kCoordinateCount};
	auto coordinates = encoder(record);
	coordinates.push_back(1.0);
	return coordinates;
}

auto solve_linear_system(std::vector<std::vector<double>> matrix, std::vector<double> rhs) -> std::vector<double>
{
	const auto size = matrix.size();
	for (std::size_t pivot = 0; pivot < size; ++pivot) {
		std::size_t best = pivot;
		for (std::size_t row = pivot + 1; row < size; ++row) {
			if (std::abs(matrix[row][pivot]) > std::abs(matrix[best][pivot])) {
				best = row;
			}
		}
		if (std::abs(matrix[best][pivot]) <= kEps) {
			matrix[best][pivot] = kEps;
		}
		if (best != pivot) {
			std::swap(matrix[best], matrix[pivot]);
			std::swap(rhs[best], rhs[pivot]);
		}
		const auto scale = matrix[pivot][pivot];
		for (std::size_t column = pivot; column < size; ++column) {
			matrix[pivot][column] /= scale;
		}
		rhs[pivot] /= scale;
		for (std::size_t row = 0; row < size; ++row) {
			if (row == pivot) {
				continue;
			}
			const auto factor = matrix[row][pivot];
			if (std::abs(factor) <= kEps) {
				continue;
			}
			for (std::size_t column = pivot; column < size; ++column) {
				matrix[row][column] -= factor * matrix[pivot][column];
			}
			rhs[row] -= factor * rhs[pivot];
		}
	}
	return rhs;
}

auto apply_linear_map(const std::vector<double> &features, const LinearCoordinateMap &map) -> std::vector<double>
{
	std::vector<double> out(map.output_dimensions, 0.0);
	for (std::size_t axis = 0; axis < map.output_dimensions; ++axis) {
		for (std::size_t feature = 0; feature < map.input_dimensions; ++feature) {
			out[axis] += features[feature] * map.weights[feature][axis];
		}
	}
	return out;
}

auto target_mse(const std::vector<std::vector<double>> &positions, const std::vector<std::vector<double>> &targets)
	-> double
{
	if (positions.empty()) {
		return 0.0;
	}
	double total = 0.0;
	std::size_t count = 0;
	for (std::size_t row = 0; row < positions.size(); ++row) {
		for (std::size_t axis = 0; axis < targets[row].size(); ++axis) {
			const auto delta = positions[row][axis] - targets[row][axis];
			total += delta * delta;
			++count;
		}
	}
	return count == 0 ? 0.0 : total / static_cast<double>(count);
}

auto fit_linear_coordinate_map(const std::vector<ProcessCurve> &records,
							   const std::vector<std::vector<double>> &targets) -> LinearCoordinateMap
{
	const auto feature_count = kCoordinateCount + 1;
	const auto dimensions = targets.empty() ? 0 : targets.front().size();
	std::vector<std::vector<double>> features;
	features.reserve(records.size());
	for (const auto &record : records) {
		features.push_back(encoded_feature_vector(record));
	}

	std::vector<std::vector<double>> normal(feature_count, std::vector<double>(feature_count, 0.0));
	std::vector<std::vector<double>> rhs(feature_count, std::vector<double>(dimensions, 0.0));
	for (std::size_t row = 0; row < features.size(); ++row) {
		for (std::size_t i = 0; i < feature_count; ++i) {
			for (std::size_t j = 0; j < feature_count; ++j) {
				normal[i][j] += features[row][i] * features[row][j];
			}
			for (std::size_t axis = 0; axis < dimensions; ++axis) {
				rhs[i][axis] += features[row][i] * targets[row][axis];
			}
		}
	}
	for (std::size_t i = 0; i < feature_count - 1; ++i) {
		normal[i][i] += kRidgeLambda;
	}

	LinearCoordinateMap map;
	map.input_dimensions = feature_count;
	map.output_dimensions = dimensions;
	map.weights.assign(feature_count, std::vector<double>(dimensions, 0.0));
	for (std::size_t axis = 0; axis < dimensions; ++axis) {
		std::vector<double> axis_rhs(feature_count, 0.0);
		for (std::size_t i = 0; i < feature_count; ++i) {
			axis_rhs[i] = rhs[i][axis];
		}
		const auto solution = solve_linear_system(normal, axis_rhs);
		for (std::size_t i = 0; i < feature_count; ++i) {
			map.weights[i][axis] = solution[i];
		}
	}

	std::vector<std::vector<double>> zero_positions(records.size(), std::vector<double>(dimensions, 0.0));
	map.initial_mse = target_mse(zero_positions, targets);
	std::vector<std::vector<double>> fitted;
	fitted.reserve(records.size());
	for (const auto &feature : features) {
		fitted.push_back(apply_linear_map(feature, map));
	}
	map.final_mse = target_mse(fitted, targets);
	return map;
}

auto transform_records(const std::vector<ProcessCurve> &records, const LinearCoordinateMap &map)
	-> std::vector<std::vector<double>>
{
	std::vector<std::vector<double>> positions;
	positions.reserve(records.size());
	for (const auto &record : records) {
		positions.push_back(apply_linear_map(encoded_feature_vector(record), map));
	}
	return positions;
}

auto preservation_diagnostics(const std::vector<std::vector<double>> &source,
							  const std::vector<std::vector<double>> &latent, std::size_t k)
	-> PreservationDiagnostics
{
	PreservationDiagnostics diagnostics;
	diagnostics.per_record_error.assign(source.size(), 0.0);
	diagnostics.source_best.assign(source.size(), 0);
	diagnostics.latent_best.assign(source.size(), 0);
	diagnostics.source_best_latent_rank.assign(source.size(), 0);
	diagnostics.source_best_distance.assign(source.size(), 0.0);
	diagnostics.latent_best_distance.assign(source.size(), 0.0);
	for (std::size_t row = 0; row < source.size(); ++row) {
		const auto source_neighbors = nearest_indices(source, row, k);
		const auto latent_neighbors = nearest_indices(latent, row, k);
		std::size_t matched = 0;
		for (const auto source_neighbor : source_neighbors) {
			if (std::find(latent_neighbors.begin(), latent_neighbors.end(), source_neighbor) != latent_neighbors.end()) {
				++matched;
			}
		}
		diagnostics.matched_neighbors += matched;
		diagnostics.possible_neighbors += source_neighbors.size();
		diagnostics.per_record_error[row] =
			source_neighbors.empty() ? 0.0 : 1.0 - static_cast<double>(matched) / static_cast<double>(source_neighbors.size());
		if (!source_neighbors.empty()) {
			diagnostics.source_best[row] = source_neighbors.front();
			diagnostics.source_best_distance[row] = source[row][source_neighbors.front()];
		}
		if (!latent_neighbors.empty()) {
			diagnostics.latent_best[row] = latent_neighbors.front();
			diagnostics.latent_best_distance[row] = latent[row][latent_neighbors.front()];
		}
		std::size_t rank = 1;
		for (std::size_t candidate = 0; candidate < latent.size(); ++candidate) {
			if (candidate == row || candidate == diagnostics.source_best[row]) {
				continue;
			}
			if (latent[row][candidate] < latent[row][diagnostics.source_best[row]]) {
				++rank;
			}
		}
		diagnostics.source_best_latent_rank[row] = rank;
	}
	diagnostics.recall = diagnostics.possible_neighbors == 0
							 ? 0.0
							 : static_cast<double>(diagnostics.matched_neighbors) /
								   static_cast<double>(diagnostics.possible_neighbors);
	diagnostics.average_row_recall = 1.0;
	if (!diagnostics.per_record_error.empty()) {
		double total_error = 0.0;
		for (const auto value : diagnostics.per_record_error) {
			total_error += value;
		}
		diagnostics.average_row_recall = 1.0 - total_error / static_cast<double>(diagnostics.per_record_error.size());
	}
	diagnostics.exact = diagnostics.matched_neighbors == diagnostics.possible_neighbors;
	return diagnostics;
}

auto out_of_sample_diagnostics(const std::vector<ProcessCurve> &records, const std::vector<ProcessCurve> &queries,
							   const std::vector<std::vector<double>> &latent,
							   const std::vector<std::vector<double>> &query_latent_positions, std::size_t k)
	-> OutOfSampleDiagnostics
{
	OutOfSampleDiagnostics diagnostics;
	diagnostics.query_count = queries.size();
	diagnostics.evaluated_neighbor_count = k;
	diagnostics.source_best.assign(queries.size(), 0);
	diagnostics.latent_best.assign(queries.size(), 0);
	diagnostics.latent_source_rank.assign(queries.size(), 1);
	diagnostics.source_best_distance.assign(queries.size(), 0.0);
	diagnostics.latent_distance.assign(queries.size(), 0.0);
	diagnostics.best_distance_penalty.assign(queries.size(), 0.0);

	AlignedCurveDistance metric;
	for (std::size_t query_index = 0; query_index < queries.size(); ++query_index) {
		std::vector<std::pair<double, std::size_t>> source_candidates;
		source_candidates.reserve(records.size());
		for (std::size_t record_index = 0; record_index < records.size(); ++record_index) {
			source_candidates.push_back({metric(queries[query_index], records[record_index]), record_index});
		}
		std::sort(source_candidates.begin(), source_candidates.end());
		diagnostics.source_best[query_index] = source_candidates.front().second;
		diagnostics.source_best_distance[query_index] = source_candidates.front().first;

		std::vector<std::pair<double, std::size_t>> latent_candidates;
		latent_candidates.reserve(records.size());
		for (std::size_t record_index = 0; record_index < records.size(); ++record_index) {
			double squared = 0.0;
			for (std::size_t axis = 0; axis < query_latent_positions[query_index].size(); ++axis) {
				const auto delta = query_latent_positions[query_index][axis] - latent[record_index][axis];
				squared += delta * delta;
			}
			latent_candidates.push_back({std::sqrt(squared), record_index});
		}
		std::sort(latent_candidates.begin(), latent_candidates.end());
		diagnostics.latent_best[query_index] = latent_candidates.front().second;
		diagnostics.latent_distance[query_index] = latent_candidates.front().first;

		std::size_t source_rank = 1;
		double mapped_source_distance = source_candidates.front().first;
		for (std::size_t rank = 0; rank < source_candidates.size(); ++rank) {
			if (source_candidates[rank].second == diagnostics.latent_best[query_index]) {
				source_rank = rank + 1;
				mapped_source_distance = source_candidates[rank].first;
				break;
			}
		}
		diagnostics.latent_source_rank[query_index] = source_rank;
		diagnostics.maximum_mapped_best_source_rank =
			std::max(diagnostics.maximum_mapped_best_source_rank, source_rank);
		diagnostics.average_mapped_best_source_rank += static_cast<double>(source_rank);

		const auto top_k = std::min(k, source_candidates.size());
		for (std::size_t rank = 0; rank < top_k; ++rank) {
			if (source_candidates[rank].second == diagnostics.latent_best[query_index]) {
				++diagnostics.anchor_matches;
				break;
			}
		}
		if (diagnostics.source_best[query_index] == diagnostics.latent_best[query_index]) {
			++diagnostics.first_anchor_matches;
		}
		const auto penalty = mapped_source_distance - source_candidates.front().first;
		diagnostics.best_distance_penalty[query_index] = penalty;
		diagnostics.average_best_distance_penalty += penalty;
		diagnostics.maximum_best_distance_penalty =
			std::max(diagnostics.maximum_best_distance_penalty, penalty);
	}
	if (!queries.empty()) {
		diagnostics.anchor_recall = static_cast<double>(diagnostics.anchor_matches) / static_cast<double>(queries.size());
		diagnostics.first_anchor_match_rate =
			static_cast<double>(diagnostics.first_anchor_matches) / static_cast<double>(queries.size());
		diagnostics.average_mapped_best_source_rank /= static_cast<double>(queries.size());
		diagnostics.average_best_distance_penalty /= static_cast<double>(queries.size());
	}
	return diagnostics;
}

auto query_diagnostics_json(const std::vector<ProcessCurve> &records, const std::vector<ProcessCurve> &queries,
							const std::vector<std::string> &ids, const std::vector<std::vector<double>> &source,
							const OutOfSampleDiagnostics &oos) -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(queries.size());
	for (std::size_t query_index = 0; query_index < queries.size(); ++query_index) {
		const auto best_source = oos.source_best[query_index];
		const auto best_latent = oos.latent_best[query_index];

		entries.push_back(json_object({
			{"query_id", visual::quote(queries[query_index].id)},
			{"query_family", visual::quote(queries[query_index].family)},
			{"source_metric_nearest_record_id", visual::quote(ids[best_source])},
			{"source_metric_nearest_family", visual::quote(records[best_source].family)},
			{"source_metric_distance", visual::num(oos.source_best_distance[query_index])},
			{"latent_nearest_record_id", visual::quote(ids[best_latent])},
			{"latent_nearest_family", visual::quote(records[best_latent].family)},
			{"latent_distance", visual::num(oos.latent_distance[query_index])},
			{"latent_best_source_metric_rank", json_size(oos.latent_source_rank[query_index])},
			{"source_metric_distance_penalty", visual::num(oos.best_distance_penalty[query_index])},
			{"native_source_metric_row_mean", visual::num(row_mean_distance(source, best_source))},
		}));
	}
	return visual::array_of(entries);
}

auto build_visual_document() -> std::string
{
	const auto records = process_records();
	const auto queries = held_out_queries();
	const auto ids = record_ids(records);

	const auto source_distances = aligned_distance_matrix(records);
	const auto targets = diffusion_coordinate_targets(source_distances, 2);
	const auto linear_map = fit_linear_coordinate_map(records, targets.coordinates);
	const auto latent_coords = transform_records(records, linear_map);
	const auto query_latent_coords = transform_records(queries, linear_map);
	const auto latent_distances = euclidean_distance_matrix(latent_coords);
	const auto source_law = diagnose_metric_law(source_distances);
	const auto latent_law = diagnose_metric_law(latent_distances);
	const auto target_coords = targets.coordinates;
	const auto preservation = preservation_diagnostics(source_distances, latent_distances, kNeighborCount);
	const auto oos_stability =
		out_of_sample_diagnostics(records, queries, latent_coords, query_latent_coords, kNeighborCount);
	const auto source_coords = source_coordinate_positions(records);
	const auto distortion = local_distortion(source_distances, latent_distances);
	const auto target_residual = target_residuals(target_coords, latent_coords);

	std::vector<std::string> family_values;
	std::vector<double> lengths;
	std::vector<double> means;
	std::vector<double> variations;
	std::vector<double> row_mean_source;
	std::vector<double> preservation_error_values;
	std::vector<double> source_best_latent_rank_values;
	family_values.reserve(records.size());
	lengths.reserve(records.size());
	means.reserve(records.size());
	variations.reserve(records.size());
	row_mean_source.reserve(records.size());
	preservation_error_values.reserve(records.size());
	source_best_latent_rank_values.reserve(records.size());
	for (std::size_t row = 0; row < records.size(); ++row) {
		family_values.push_back(records[row].family);
		lengths.push_back(static_cast<double>(records[row].values.size()));
		means.push_back(curve_mean(records[row]));
		variations.push_back(curve_total_variation(records[row]));
		row_mean_source.push_back(row_mean_distance(source_distances, row));
		preservation_error_values.push_back(preservation.per_record_error[row]);
		source_best_latent_rank_values.push_back(static_cast<double>(preservation.source_best_latent_rank[row]));
	}

	std::vector<std::string> datasets;
	datasets.push_back(json_object({
		{"id", visual::quote(kDatasetId)},
		{"title", visual::quote("Mapping And Dimensionality Process Curves")},
		{"description", visual::quote("Native C++ scale evidence for process-curve mapping and dimensionality inspection.")},
		{"source", visual::quote("deterministic native process-curve family/variant generator")},
		{"license", visual::quote("MPL-2.0")},
	}));

	std::vector<std::string> record_entries;
	record_entries.reserve(records.size());
	for (std::size_t row = 0; row < records.size(); ++row) {
		const auto &record = records[row];
		const auto source_best = preservation.source_best[row];
		const auto latent_best = preservation.latent_best[row];
		const auto mapping_diagnostics = json_object({
			{"source_row_mean_distance", visual::num(row_mean_source[row])},
			{"local_mapping_distortion", visual::num(distortion[row])},
			{"diffusion_coordinate_target_residual", visual::num(target_residual[row])},
			{"neighborhood_preservation_error", visual::num(preservation.per_record_error[row])},
			{"source_nearest_record_id", visual::quote(ids[source_best])},
			{"source_nearest_family", visual::quote(records[source_best].family)},
			{"source_nearest_distance", visual::num(preservation.source_best_distance[row])},
			{"latent_nearest_record_id", visual::quote(ids[latent_best])},
			{"latent_nearest_family", visual::quote(records[latent_best].family)},
			{"latent_nearest_distance", visual::num(preservation.latent_best_distance[row])},
			{"source_nearest_latent_rank", json_size(preservation.source_best_latent_rank[row])},
			{"source_coordinate", json_number_array(source_coords[row])},
			{"diffusion_target_coordinate", json_number_array(target_coords[row])},
			{"parametric_latent_coordinate", json_number_array(latent_coords[row])},
		});
		const auto payload = json_object({{"kind", visual::quote("time_series")},
										  {"series", json_number_array(record.values)},
										  {"sample_rate_hz", visual::num(1.0)},
										  {"family", visual::quote(record.family)},
										  {"family_label", visual::quote(record.family_label)},
										  {"family_index", json_size(record.family_index)},
										  {"variant_index", json_size(record.variant_index)},
										  {"mapping_diagnostics", mapping_diagnostics},
										  {"native_source", visual::quote("deterministic native process-curve family/variant generator")}});
		record_entries.push_back(json_object({{"id", visual::quote(record.id)},
											  {"dataset_id", visual::quote(kDatasetId)},
											  {"record_type", visual::quote("process_curve")},
											  {"label", visual::quote(record.id + " / " + record.family_label)},
											  {"payload", payload}}));
	}

	std::vector<std::string> relations;
	relations.push_back(relation_json(
		kSourceRelationId, "aligned process-curve metric", "metric", ids, source_distances,
		json_object({{"symmetric", json_bool(true)},
					 {"complete", json_bool(true)},
					 {"storage_note", visual::quote("dense numeric matrix for symmetric metric rendering")},
					 {"source", visual::quote("AlignedCurveDistance over deterministic native process curves")},
					 {"law_check", metric_law_json(source_law, "AlignedCurveDistance")}})));
	relations.push_back(relation_json(
		kLatentRelationId, "latent Euclidean metric after parametric diffusion coordinate", "metric", ids, latent_distances,
		json_object({{"symmetric", json_bool(true)},
					 {"complete", json_bool(true)},
					 {"storage_note", visual::quote("dense numeric matrix for symmetric metric rendering")},
					 {"source", visual::quote("Euclidean metric on native mapped coordinates")},
					 {"mapping", visual::quote(linear_map.mapping)},
					 {"strategy", visual::quote(linear_map.strategy)},
					 {"law_check", metric_law_json(latent_law, "mtrc::Euclidean<double>")}})));

	std::vector<std::string> spaces;
	spaces.push_back(space_json(kSourceSpaceId, kSourceRelationId, "finite_metric_space", ids,
								json_object({{"record_count", json_size(records.size())},
											 {"metric_operator", visual::quote("AlignedCurveDistance")},
											 {"native_mapping_input", json_bool(true)}})));
	spaces.push_back(space_json(kLatentSpaceId, kLatentRelationId, "derived_coordinate_space", ids,
								json_object({{"source_space_id", visual::quote(kSourceSpaceId)},
											 {"mapping", visual::quote(linear_map.mapping)},
											 {"strategy", visual::quote(linear_map.strategy)},
											 {"inverse_supported", json_bool(false)},
											 {"out_of_sample_supported", json_bool(true)},
											 {"source_record_count", json_size(records.size())}})));

	std::vector<std::string> properties;
	properties.push_back(categorical_property_json(
		"process-family", "process family", ids, family_values,
		json_object({{"source", visual::quote("native ProcessCurve.family")}})));
	properties.push_back(scalar_property_json("curve-length", "curve sample count", ids, lengths));
	properties.push_back(scalar_property_json("curve-mean", "curve mean", ids, means));
	properties.push_back(scalar_property_json("curve-total-variation", "curve total variation", ids, variations));
	properties.push_back(scalar_property_json(
		"source-row-mean-distance", "mean aligned-curve distance", ids, row_mean_source,
		json_object({{"relation_id", visual::quote(kSourceRelationId)}})));
	properties.push_back(scalar_property_json(
		"local-mapping-distortion", "local normalized distance distortion", ids, distortion,
		json_object({{"source_relation_id", visual::quote(kSourceRelationId)},
					 {"mapped_relation_id", visual::quote(kLatentRelationId)},
					 {"calculation", visual::quote("mean absolute normalized pairwise distance difference per row")}})));
	properties.push_back(scalar_property_json(
		"diffusion-coordinate-target-residual", "diffusion-coordinate target residual", ids, target_residual,
		json_object({{"target_generator", visual::quote(targets.method)},
					 {"calculation", visual::quote("Euclidean distance between target coordinate and mapped latent coordinate")}})));
	properties.push_back(scalar_property_json(
		"neighborhood-preservation-error", "neighborhood preservation error", ids, preservation_error_values,
		json_object({{"source_relation_id", visual::quote(kSourceRelationId)},
					 {"mapped_relation_id", visual::quote(kLatentRelationId)},
					 {"neighbor_count", json_size(kNeighborCount)},
					 {"calculation", visual::quote("1 - native mapped/source k-neighbor overlap per record")}})));
	properties.push_back(scalar_property_json(
		"source-nearest-latent-rank", "source nearest-neighbor rank in latent map", ids,
		source_best_latent_rank_values,
		json_object({{"source_relation_id", visual::quote(kSourceRelationId)},
					 {"mapped_relation_id", visual::quote(kLatentRelationId)},
					 {"calculation", visual::quote("native rank of each source nearest neighbor in latent Euclidean distances")}})));

	std::vector<std::string> coordinates;
	coordinates.push_back(coordinate_json(
		"source-coordinate-layout-3d", kSourceSpaceId, "native record-coordinate codec layout", ids, source_coords,
		json_object({{"source", visual::quote("CurveCoordinateEncoder used by native record_coordinate_codec")},
					 {"metric_embedding", json_bool(false)},
					 {"purpose", visual::quote("original/native coordinate state for inspecting source records")}})));
	coordinates.push_back(coordinate_json(
		"diffusion-coordinate-target-2d", kLatentSpaceId, "native diffusion-coordinate target", ids, target_coords,
		json_object({{"target_generator", visual::quote(targets.method)},
					 {"pairwise_distances", visual::quote(targets.pairwise_distances)},
					 {"affinity_kernel", visual::quote(targets.affinity_kernel)},
					 {"diffusion_operator", visual::quote(targets.diffusion_operator)},
					 {"kernel_scale", visual::num(targets.kernel_scale)},
					 {"anchor_record_ids", visual::string_array({ids[targets.anchor_indices[0]], ids[targets.anchor_indices[1]]})}})));
	coordinates.push_back(coordinate_json(
		"parametric-coordinate-latent-2d", kLatentSpaceId, "parametric diffusion coordinate latent map", ids, latent_coords,
		json_object({{"mapping", visual::quote(linear_map.mapping)},
					 {"strategy", visual::quote(linear_map.strategy)},
					 {"solver", visual::quote("native_ridge_linear_coordinate_solver")},
					 {"ridge_lambda", visual::num(kRidgeLambda)},
					 {"input_dimensions", json_size(linear_map.input_dimensions)},
					 {"target_dimensions", json_size(linear_map.output_dimensions)}})));

	std::vector<std::string> graphs;
	graphs.push_back(knn_graph_json(ids, source_distances));

	std::vector<std::string> morph_steps;
	morph_steps.push_back(json_object({{"id", visual::quote("mapping-coordinate-morph-00")},
									   {"index", json_size(0)},
									   {"t", visual::num(0.0)},
									   {"name", visual::quote("source process-curve coordinate layout")},
									   {"coordinate_id", visual::quote("source-coordinate-layout-3d")},
									   {"relation_id", visual::quote(kSourceRelationId)},
									   {"property_id", visual::quote("process-family")}}));
	morph_steps.push_back(json_object({{"id", visual::quote("mapping-coordinate-morph-01")},
									   {"index", json_size(1)},
									   {"t", visual::num(0.5)},
									   {"name", visual::quote("native diffusion-coordinate target")},
									   {"coordinate_id", visual::quote("diffusion-coordinate-target-2d")},
									   {"relation_id", visual::quote(kSourceRelationId)},
									   {"property_id", visual::quote("diffusion-coordinate-target-residual")}}));
	morph_steps.push_back(json_object({{"id", visual::quote("mapping-coordinate-morph-02")},
									   {"index", json_size(2)},
									   {"t", visual::num(1.0)},
									   {"name", visual::quote("parametric diffusion-coordinate latent map")},
									   {"coordinate_id", visual::quote("parametric-coordinate-latent-2d")},
									   {"relation_id", visual::quote(kLatentRelationId)},
									   {"property_id", visual::quote("local-mapping-distortion")}}));
	std::vector<std::string> timelines;
	timelines.push_back(json_object({{"id", visual::quote(kMorphTimelineId)},
									 {"dataset_id", visual::quote(kDatasetId)},
									 {"name", visual::quote("source to diffusion target to parametric latent coordinate morph")},
									 {"space_id", visual::quote(kLatentSpaceId)},
									 {"timeline_type", visual::quote("mapping_coordinate_morph")},
									 {"steps", visual::array_of(morph_steps)},
									 {"metadata",
									  json_object({{"motion_grammar", visual::quote("mapping-coordinate-morph")},
												   {"source_coordinate_id", visual::quote("source-coordinate-layout-3d")},
												   {"target_coordinate_id", visual::quote("diffusion-coordinate-target-2d")},
												   {"latent_coordinate_id", visual::quote("parametric-coordinate-latent-2d")},
												   {"residual_property_id", visual::quote("local-mapping-distortion")},
												   {"algorithmic_computation", json_bool(false)}})}}));

	std::vector<std::string> diagnostics;
	diagnostics.push_back(json_object({
		{"id", visual::quote("native-pipeline-path")},
		{"dataset_id", visual::quote(kDatasetId)},
		{"diagnostic_type", visual::quote("native-mapping-pipeline")},
		{"payload", json_object({{"source_examples", visual::string_array({"examples/engine/relation_matrix_visual_export.cpp",
																			"examples/engine/process_curves_space.cpp",
																			"examples/engine/metric_space_mapping_pipeline.cpp"})},
								 {"pipeline", visual::quote("native_process_curve_diffusion_coordinate_scale_export")},
								 {"component_count", json_size(7)},
								 {"codec", visual::quote("CurveCoordinateEncoder")},
								 {"pairwise_distances", visual::quote(targets.pairwise_distances)},
								 {"affinity_kernel", visual::quote(targets.affinity_kernel)},
								 {"diffusion_operator", visual::quote(targets.diffusion_operator)},
								 {"target_method", visual::quote(targets.method)},
								 {"target_dimensions", json_size(linear_map.output_dimensions)},
								 {"record_count", json_size(records.size())},
								 {"relation_storage", visual::quote("symmetric_dense_matrix dense numeric matrix")},
								 {"dense_distance_evaluations", json_size(targets.dense_distance_evaluations)}})}
	}));
	diagnostics.push_back(json_object({
		{"id", visual::quote("native-coordinate-calibration-report")},
		{"dataset_id", visual::quote(kDatasetId)},
		{"diagnostic_type", visual::quote("coordinate-calibration")},
		{"payload", json_object({{"calibration_steps", json_size(1)},
								 {"calibration_seed", json_size(0)},
								 {"batch_size", json_size(records.size())},
								 {"solver", visual::quote("native_ridge_linear_coordinate_solver")},
								 {"ridge_lambda", visual::num(kRidgeLambda)},
								 {"initial_coordinate_target_mse", visual::num(linear_map.initial_mse)},
								 {"final_coordinate_target_mse", visual::num(linear_map.final_mse)},
								 {"report", visual::quote("native_closed_form_coordinate_fit_report")}})}
	}));
	diagnostics.push_back(json_object({
		{"id", visual::quote("neighbor-preservation")},
		{"dataset_id", visual::quote(kDatasetId)},
		{"diagnostic_type", visual::quote("neighbor_preservation")},
		{"payload", json_object({{"source_record_count", json_size(records.size())},
								 {"mapped_record_count", json_size(latent_coords.size())},
								 {"requested_neighbor_count", json_size(kNeighborCount)},
								 {"evaluated_neighbor_count", json_size(kNeighborCount)},
								 {"matched_neighbors", json_size(preservation.matched_neighbors)},
								 {"possible_neighbors", json_size(preservation.possible_neighbors)},
								 {"recall", visual::num(preservation.recall)},
								 {"average_row_recall", visual::num(preservation.average_row_recall)},
								 {"exact", json_bool(preservation.exact)}})}
	}));
	diagnostics.push_back(json_object({
		{"id", visual::quote("out-of-sample-stability")},
		{"dataset_id", visual::quote(kDatasetId)},
		{"diagnostic_type", visual::quote("out_of_sample_neighbor_stability")},
		{"payload", json_object({{"source_record_count", json_size(records.size())},
								 {"query_record_count", json_size(oos_stability.query_count)},
								 {"evaluated_neighbor_count", json_size(oos_stability.evaluated_neighbor_count)},
								 {"anchor_recall", visual::num(oos_stability.anchor_recall)},
								 {"first_anchor_match_rate", visual::num(oos_stability.first_anchor_match_rate)},
								 {"average_mapped_best_source_rank", visual::num(oos_stability.average_mapped_best_source_rank)},
								 {"maximum_mapped_best_source_rank", json_size(oos_stability.maximum_mapped_best_source_rank)},
								 {"average_best_distance_penalty", visual::num(oos_stability.average_best_distance_penalty)},
								 {"maximum_best_distance_penalty", visual::num(oos_stability.maximum_best_distance_penalty)},
								 {"transform_supported", json_bool(true)},
								 {"query_nearest_records", query_diagnostics_json(records, queries, ids, source_distances,
																				  oos_stability)}})}
	}));

	std::vector<std::string> views;
	views.push_back(json_object({{"id", visual::quote("mapping-dimensionality-source-view")},
								 {"kind", visual::quote("metric-space")},
								 {"name", visual::quote("Process-curve source metric space")},
								 {"spaceId", visual::quote(kSourceSpaceId)},
								 {"coordinateId", visual::quote("source-coordinate-layout-3d")},
								 {"propertyId", visual::quote("process-family")}}));
	views.push_back(json_object({{"id", visual::quote("mapping-dimensionality-latent-view")},
								 {"kind", visual::quote("metric-space")},
								 {"name", visual::quote("Native parametric diffusion coordinate latent map")},
								 {"spaceId", visual::quote(kLatentSpaceId)},
								 {"coordinateId", visual::quote("parametric-coordinate-latent-2d")},
								 {"propertyId", visual::quote("local-mapping-distortion")}}));
	views.push_back(json_object({{"id", visual::quote("mapping-dimensionality-coordinate-morph-view")},
								 {"kind", visual::quote("mapping")},
								 {"name", visual::quote("Source to diffusion target to parametric latent map")},
								 {"spaceId", visual::quote(kLatentSpaceId)},
								 {"sourceCoordinateId", visual::quote("parametric-coordinate-latent-2d")},
								 {"targetCoordinateId", visual::quote("source-coordinate-layout-3d")},
								 {"diffusionTargetCoordinateId", visual::quote("diffusion-coordinate-target-2d")},
								 {"residualProperty", visual::quote("local-mapping-distortion")},
								 {"residualPropertyId", visual::quote("local-mapping-distortion")},
								 {"preservationErrorPropertyId", visual::quote("neighborhood-preservation-error")},
								 {"graphId", visual::quote(kGraphId)},
								 {"timelineId", visual::quote(kMorphTimelineId)}}));
	views.push_back(json_object({{"id", visual::quote("mapping-dimensionality-relation-view")},
								 {"kind", visual::quote("relation-matrix")},
								 {"name", visual::quote("Aligned process-curve distance matrix")},
								 {"relationId", visual::quote(kSourceRelationId)}}));

	visual::Document document;
	document.provenance_json(json_object(
		{{"generator", visual::quote("examples/engine/mapping_dimensionality_visual_export.cpp")},
		 {"runtime", visual::quote("native C++")},
		 {"native_export", json_bool(true)},
		 {"algorithm_source", visual::quote("native process-curve diffusion coordinate mapping scale pipeline")},
		 {"source_examples", visual::string_array({"examples/engine/relation_matrix_visual_export.cpp",
												   "examples/engine/process_curve_diffusion_coordinate_gallery.cpp",
												   "examples/engine/metric_space_mapping_pipeline.cpp",
												   "examples/engine/parametric_diffusion_coordinate_pipeline_builder.cpp"})},
		 {"synthetic_js", json_bool(false)}}));
	for (const auto &dataset : datasets) {
		document.dataset_json(dataset);
	}
	for (const auto &record : record_entries) {
		document.record_json(record);
	}
	for (const auto &relation : relations) {
		document.relation_json(relation);
	}
	for (const auto &space : spaces) {
		document.space_json(space);
	}
	for (const auto &property : properties) {
		document.property_json(property);
	}
	for (const auto &graph : graphs) {
		document.graph_json(graph);
	}
	for (const auto &coordinate : coordinates) {
		document.coordinates_json(coordinate);
	}
	for (const auto &timeline : timelines) {
		document.timeline_json(timeline);
	}
	for (const auto &view : views) {
		document.view_json(view);
	}
	for (const auto &diagnostic : diagnostics) {
		document.diagnostic_json(diagnostic);
	}
	return document.to_json();
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
		for (int index = 1; index < argc; ++index) {
			const std::string arg = argv[index];
			if (arg == "--help" || arg == "-h") {
				std::cout << usage(argv[0]);
				return 0;
			}
			if (arg == "--export-dir") {
				if (index + 1 >= argc) {
					throw std::invalid_argument("--export-dir requires a directory argument");
				}
				export_dir = argv[++index];
				continue;
			}
			throw std::invalid_argument("unknown argument: " + arg);
		}

		const auto json = build_visual_document();
		if (export_dir.empty()) {
			std::cout << json << "\n";
			return 0;
		}

		const auto output_path = export_dir / "metric.visual.json";
		if (!visual::write_metric_visual_file(export_dir, json + "\n")) {
			throw std::runtime_error("failed to write output file: " + output_path.string());
		}
		std::cerr << "wrote " << output_path << "\n";
		return 0;
	} catch (const std::exception &error) {
		std::cerr << "mapping_dimensionality_visual_export: " << error.what() << "\n";
		std::cerr << usage(argv[0]);
		return 1;
	}
}
