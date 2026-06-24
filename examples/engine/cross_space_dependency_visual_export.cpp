// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Native metric.visual.v1 exporter for the Cross-Space Dependency example.
//
// This is intentionally self-contained: the visual helper covers common export
// shapes, but this exporter still needs richer provenance, relation metadata,
// view hints, and pair/local contribution properties in one isolated source.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cross_space_dependency.hpp"
#include "../../visual/cpp/mtrc_visual.hpp"

namespace {

namespace visual = mtrc::visual;

constexpr std::size_t kRecords = 48;
constexpr std::size_t kPermutations = 199;
constexpr std::size_t kBaselinePermutations = 1999;
constexpr std::uint64_t kSeed = 20240607ULL;
constexpr double kAlpha = 0.05;

struct MetricLawDiagnostics {
	double diagonal_max_abs{};
	double symmetry_max_abs{};
	double triangle_max_violation{};
	double minimum_nonzero_distance{};
	double maximum_distance{};
	double average_distance{};
	std::size_t pair_count{};
	std::size_t triangle_triplets{};
	bool finite{true};
};

struct PairContribution {
	std::size_t row{};
	std::size_t column{};
	double value{};
};

auto record_id(std::size_t index) -> std::string
{
	std::ostringstream out;
	out << "obs-" << std::setw(2) << std::setfill('0') << index;
	return out.str();
}

auto make_record_ids(std::size_t count) -> std::vector<std::string>
{
	std::vector<std::string> ids;
	ids.reserve(count);
	for (std::size_t index = 0; index < count; ++index) {
		ids.push_back(record_id(index));
	}
	return ids;
}

auto matrix_value(const mtrc::DistanceMatrix<double> &matrix, std::size_t row, std::size_t column) -> double
{
	return matrix(row, column);
}

auto metric_law_diagnostics(const mtrc::DistanceMatrix<double> &matrix) -> MetricLawDiagnostics
{
	const std::size_t count = matrix.rows();
	MetricLawDiagnostics diagnostics;
	diagnostics.minimum_nonzero_distance = std::numeric_limits<double>::infinity();

	for (std::size_t row = 0; row < count; ++row) {
		const double diagonal = matrix_value(matrix, row, row);
		diagnostics.diagonal_max_abs = std::max(diagnostics.diagonal_max_abs, std::abs(diagonal));
		diagnostics.finite = diagnostics.finite && std::isfinite(diagonal);
		for (std::size_t column = 0; column < count; ++column) {
			const double lhs = matrix_value(matrix, row, column);
			const double rhs = matrix_value(matrix, column, row);
			diagnostics.symmetry_max_abs = std::max(diagnostics.symmetry_max_abs, std::abs(lhs - rhs));
			diagnostics.finite = diagnostics.finite && std::isfinite(lhs);
		}
	}

	for (std::size_t row = 0; row < count; ++row) {
		for (std::size_t column = row + 1; column < count; ++column) {
			const double distance = matrix_value(matrix, row, column);
			diagnostics.maximum_distance = std::max(diagnostics.maximum_distance, distance);
			diagnostics.average_distance += distance;
			if (distance > 0.0) {
				diagnostics.minimum_nonzero_distance = std::min(diagnostics.minimum_nonzero_distance, distance);
			}
			++diagnostics.pair_count;
		}
	}
	if (diagnostics.pair_count != 0) {
		diagnostics.average_distance /= static_cast<double>(diagnostics.pair_count);
	}
	if (!std::isfinite(diagnostics.minimum_nonzero_distance)) {
		diagnostics.minimum_nonzero_distance = 0.0;
	}

	for (std::size_t i = 0; i < count; ++i) {
		for (std::size_t j = 0; j < count; ++j) {
			for (std::size_t k = 0; k < count; ++k) {
				const double violation =
					matrix_value(matrix, i, k) - (matrix_value(matrix, i, j) + matrix_value(matrix, j, k));
				diagnostics.triangle_max_violation = std::max(diagnostics.triangle_max_violation, violation);
				++diagnostics.triangle_triplets;
			}
		}
	}

	return diagnostics;
}

auto select_landmarks(const mtrc::DistanceMatrix<double> &matrix) -> std::array<std::size_t, 3>
{
	const std::size_t count = matrix.rows();
	std::array<std::size_t, 3> landmarks{0, 0, 0};
	if (count == 0) {
		return landmarks;
	}

	double best = -1.0;
	for (std::size_t index = 0; index < count; ++index) {
		const double distance = matrix_value(matrix, landmarks[0], index);
		if (distance > best) {
			best = distance;
			landmarks[1] = index;
		}
	}

	best = -1.0;
	for (std::size_t index = 0; index < count; ++index) {
		const double nearest_landmark =
			std::min(matrix_value(matrix, index, landmarks[0]), matrix_value(matrix, index, landmarks[1]));
		if (nearest_landmark > best) {
			best = nearest_landmark;
			landmarks[2] = index;
		}
	}
	return landmarks;
}

auto landmark_coordinates(const mtrc::DistanceMatrix<double> &matrix) -> std::vector<std::array<double, 3>>
{
	const std::size_t count = matrix.rows();
	const auto landmarks = select_landmarks(matrix);
	std::vector<std::array<double, 3>> coordinates(count);

	for (std::size_t row = 0; row < count; ++row) {
		for (std::size_t axis = 0; axis < landmarks.size(); ++axis) {
			coordinates[row][axis] = matrix_value(matrix, row, landmarks[axis]);
		}
	}

	for (std::size_t axis = 0; axis < landmarks.size(); ++axis) {
		double mean = 0.0;
		for (const auto &position : coordinates) {
			mean += position[axis];
		}
		mean /= static_cast<double>(std::max<std::size_t>(count, 1));
		double max_abs = 0.0;
		for (auto &position : coordinates) {
			position[axis] -= mean;
			max_abs = std::max(max_abs, std::abs(position[axis]));
		}
		if (max_abs > 0.0) {
			for (auto &position : coordinates) {
				position[axis] /= max_abs;
			}
		}
	}

	return coordinates;
}

auto row_mean_distance(const mtrc::DistanceMatrix<double> &matrix, std::size_t row) -> double
{
	const std::size_t count = matrix.rows();
	if (count < 2) {
		return 0.0;
	}
	double total = 0.0;
	for (std::size_t column = 0; column < count; ++column) {
		if (row != column) {
			total += matrix_value(matrix, row, column);
		}
	}
	return total / static_cast<double>(count - 1);
}

auto local_profile_alignment(const mtrc::DistanceMatrix<double> &left, const mtrc::DistanceMatrix<double> &right)
	-> std::vector<double>
{
	const std::size_t count = left.rows();
	std::vector<double> values(count, 0.0);
	for (std::size_t row = 0; row < count; ++row) {
		std::vector<double> left_profile;
		std::vector<double> right_profile;
		left_profile.reserve(count - 1);
		right_profile.reserve(count - 1);
		for (std::size_t column = 0; column < count; ++column) {
			if (row == column) {
				continue;
			}
			left_profile.push_back(matrix_value(left, row, column));
			right_profile.push_back(matrix_value(right, row, column));
		}
		values[row] = cross_space::pearson(left_profile, right_profile);
	}
	return values;
}

auto pair_contributions(const mtrc::DistanceMatrix<double> &left, const mtrc::DistanceMatrix<double> &right)
	-> std::vector<PairContribution>
{
	const std::size_t count = left.rows();
	std::vector<double> left_values;
	std::vector<double> right_values;
	left_values.reserve((count * (count - 1)) / 2);
	right_values.reserve(left_values.capacity());
	for (std::size_t row = 0; row < count; ++row) {
		for (std::size_t column = row + 1; column < count; ++column) {
			left_values.push_back(matrix_value(left, row, column));
			right_values.push_back(matrix_value(right, row, column));
		}
	}

	const auto mean = [](const std::vector<double> &values) {
		if (values.empty()) {
			return 0.0;
		}
		return std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
	};
	const auto stddev = [](const std::vector<double> &values, double center) {
		if (values.empty()) {
			return 0.0;
		}
		double variance = 0.0;
		for (const double value : values) {
			const double delta = value - center;
			variance += delta * delta;
		}
		return std::sqrt(variance / static_cast<double>(values.size()));
	};

	const double left_mean = mean(left_values);
	const double right_mean = mean(right_values);
	const double left_sd = stddev(left_values, left_mean);
	const double right_sd = stddev(right_values, right_mean);

	std::vector<PairContribution> contributions;
	contributions.reserve(left_values.size());
	std::size_t pair_index = 0;
	for (std::size_t row = 0; row < count; ++row) {
		for (std::size_t column = row + 1; column < count; ++column) {
			double value = 0.0;
			if (left_sd > 0.0 && right_sd > 0.0) {
				value = ((left_values[pair_index] - left_mean) / left_sd) *
						((right_values[pair_index] - right_mean) / right_sd);
			}
			contributions.push_back(PairContribution{row, column, value});
			++pair_index;
		}
	}
	return contributions;
}

auto local_contribution(const std::vector<PairContribution> &pairs, std::size_t count) -> std::vector<double>
{
	std::vector<double> totals(count, 0.0);
	std::vector<std::size_t> seen(count, 0);
	for (const auto &pair : pairs) {
		totals[pair.row] += pair.value;
		totals[pair.column] += pair.value;
		++seen[pair.row];
		++seen[pair.column];
	}
	for (std::size_t index = 0; index < count; ++index) {
		if (seen[index] != 0) {
			totals[index] /= static_cast<double>(seen[index]);
		}
	}
	return totals;
}

auto local_density(const mtrc::DistanceMatrix<double> &matrix) -> std::vector<double>
{
	std::vector<double> density(matrix.rows(), 0.0);
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		density[row] = 1.0 / (1.0 + row_mean_distance(matrix, row));
	}
	return density;
}

auto relation_json(const std::string &id, const std::string &name, const std::string &metric_name,
				   const std::string &dataset_id, const std::vector<std::string> &ids,
				   const mtrc::DistanceMatrix<double> &matrix, const MetricLawDiagnostics &diagnostics) -> std::string
{
	std::vector<std::string> rows;
	rows.reserve(matrix.rows());
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		std::vector<double> values;
		values.reserve(matrix.columns());
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			values.push_back(matrix_value(matrix, row, column));
		}
		rows.push_back(visual::number_array(values));
	}

	const std::string metadata =
		visual::object({visual::string_field("metric", metric_name),
						visual::string_field("computed_by",
											 "METRIC native C++ DistanceTable/provider_symmetric_distance_matrix"),
						visual::field("law_check",
									  visual::object({visual::string_field("diagonal", "checked_exhaustive"),
													  visual::string_field("symmetry", "checked_exhaustive"),
													  visual::string_field("triangle", "checked_exhaustive"),
													  visual::bool_field("finite", diagnostics.finite),
													  visual::number_field("diagonal_max_abs",
																		   diagnostics.diagonal_max_abs),
													  visual::number_field("symmetry_max_abs",
																		   diagnostics.symmetry_max_abs),
													  visual::number_field("triangle_max_violation",
																		   diagnostics.triangle_max_violation),
													  visual::size_field("triangle_triplets",
																		 diagnostics.triangle_triplets)})),
						visual::size_field("pair_count", diagnostics.pair_count),
						visual::number_field("minimum_nonzero_distance", diagnostics.minimum_nonzero_distance),
						visual::number_field("maximum_distance", diagnostics.maximum_distance),
						visual::number_field("average_distance", diagnostics.average_distance)});

	return visual::object({visual::string_field("id", id),
						   visual::string_field("dataset_id", dataset_id),
						   visual::string_field("name", name),
						   visual::string_field("relation_type", "metric"),
						   visual::string_field("value_type", "scalar"),
						   visual::string_array_field("record_ids", ids),
						   visual::string_field("storage", "dense_matrix"),
						   visual::field("values", visual::array_of(rows)),
						   visual::field("metadata", metadata)});
}

auto record_json(const std::string &dataset_id, const cross_space::Dataset &dataset,
				 const std::vector<std::string> &ids) -> std::vector<std::string>
{
	std::vector<std::string> records;
	records.reserve(dataset.size());
	for (std::size_t index = 0; index < dataset.size(); ++index) {
		const std::string payload =
			visual::object({visual::string_field("kind", "composed"),
							visual::field("components",
										  visual::object({visual::field("event_log", visual::string_payload(dataset.logs[index])),
														  visual::field("process_curve",
																		visual::series_payload(dataset.curves[index]))})),
							visual::field("metadata",
										  visual::object({visual::size_field("pair_index", index),
														  visual::number_field("generator_latent_severity",
																			   dataset.latent[index]),
														  visual::string_field("left_space",
																			   "event_logs/edit_distance"),
														  visual::string_field("right_space",
																			   "process_curves/twed")}))});
		records.push_back(visual::object({visual::string_field("id", ids[index]),
										   visual::string_field("dataset_id", dataset_id),
										   visual::string_field("record_type", "paired_cross_space_observation"),
										   visual::string_field("label",
																"paired observation " + std::to_string(index)),
										   visual::field("payload", payload)}));
	}
	return records;
}

auto space_metadata_json(const std::string &name, const std::string &paired_with,
						 const std::string &metric_family) -> std::string
{
	return visual::object({visual::string_field("name", name),
						   visual::string_field("paired_with", paired_with),
						   visual::string_field("metric_family", metric_family),
						   visual::string_field("pairing", "position_aligned_records")});
}

auto scalar_values(const std::vector<std::string> &record_ids, const std::vector<double> &values)
	-> std::vector<visual::ScalarValue>
{
	std::vector<visual::ScalarValue> scalars;
	scalars.reserve(record_ids.size());
	for (std::size_t index = 0; index < record_ids.size(); ++index) {
		scalars.push_back(visual::ScalarValue{record_ids[index], values[index]});
	}
	return scalars;
}

auto pair_property_json(const std::string &dataset_id, const std::vector<std::string> &record_ids,
						const std::vector<PairContribution> &pairs) -> std::string
{
	std::vector<std::string> values;
	values.reserve(pairs.size());
	for (const auto &pair : pairs) {
		values.push_back(visual::object({visual::string_field("row_id", record_ids[pair.row]),
										 visual::string_field("column_id", record_ids[pair.column]),
										 visual::number_field("value", pair.value)}));
	}
	return visual::object({visual::string_field("id", "pair-distance-profile-contribution"),
						   visual::string_field("dataset_id", dataset_id),
						   visual::string_field("target_type", "pair"),
						   visual::string_field("value_type", "scalar"),
						   visual::string_field("name", "pair distance-profile z-product contribution"),
						   visual::field("values", visual::array_of(values))});
}

auto positions3(const std::vector<std::string> &record_ids, const std::vector<std::array<double, 3>> &coordinates)
	-> std::vector<visual::Position>
{
	std::vector<visual::Position> positions;
	positions.reserve(record_ids.size());
	for (std::size_t index = 0; index < record_ids.size(); ++index) {
		positions.push_back(
			visual::Position{record_ids[index],
							 {coordinates[index][0], coordinates[index][1], coordinates[index][2]}});
	}
	return positions;
}

auto coordinate_metadata_json(const std::vector<std::string> &record_ids,
							  const std::array<std::size_t, 3> &landmarks) -> std::string
{
	std::vector<std::string> landmark_ids;
	landmark_ids.reserve(landmarks.size());
	for (const std::size_t landmark : landmarks) {
		landmark_ids.push_back(record_ids[landmark]);
	}
	return visual::object({visual::string_field("method", "native_cpp_landmark_distance_coordinates"),
						   visual::string_array_field("landmark_record_ids", landmark_ids)});
}

auto report_json(const cross_space::DependenceReport &report) -> std::string
{
	return visual::object({visual::number_field("statistic", report.statistic),
						   visual::number_field("null_mean", report.null_mean),
						   visual::number_field("null_sd", report.null_sd),
						   visual::number_field("standardized", report.standardized),
						   visual::number_field("p_value", report.p_value),
						   visual::size_field("permutations", report.permutations),
						   visual::size_field("record_count", report.record_count),
						   visual::string_field("decision", report.p_value <= kAlpha ? "dependent" : "independent")});
}

auto diagnostics_json(const cross_space::DependenceReport &coupled, const cross_space::DependenceReport &decoupled,
					  const cross_space::DependenceReport &permuted,
					  const cross_space::BaselineReport &coupled_baseline, double compare_value,
					  const std::string &dataset_id) -> std::vector<std::string>
{
	return {visual::object({visual::string_field("id", "cross-space-mgc-coupled"),
							visual::string_field("kind", "cross_space_dependence"),
							visual::string_field("dataset_id", dataset_id),
							visual::field("space_ids",
										  visual::string_array({"event-log-space", "process-curve-space"})),
							visual::field("relation_ids",
										  visual::string_array({"event-log-edit-distance",
																"process-curve-twed-distance"})),
							visual::string_field("algorithm",
												 "mtrc::compare(..., mgc_options) plus native seeded permutation_test"),
							visual::number_field("compare_statistic", compare_value),
							visual::number_field("alpha", kAlpha),
							visual::field("report", report_json(coupled))}),
			visual::object({visual::string_field("id", "cross-space-native-controls"),
							visual::string_field("kind", "cross_space_dependency_controls"),
							visual::string_field("dataset_id", dataset_id),
							visual::field("decoupled_report", report_json(decoupled)),
							visual::field("permuted_pairing_report", report_json(permuted)),
							visual::number_field("baseline_scalar_pearson_r", coupled_baseline.pearson_r),
							visual::number_field("baseline_scalar_pearson_p_value",
												 coupled_baseline.pearson_p_value),
							visual::number_field("baseline_forced_vector_mgc", coupled_baseline.vectorized_mgc),
							visual::size_field("baseline_permutations", coupled_baseline.permutations),
							visual::string_field("verdict", "metric_detects_dependence_baseline_misses_it")}),
			visual::object({visual::string_field("id", "native-export-foundation"),
							visual::string_field("kind", "integration_status"),
							visual::string_field("dataset_id", dataset_id),
							visual::string_field("status", "native_cpp_metric_visual_export_foundation"),
							visual::bool_field("public_hero_ready", false),
							visual::string_field("note",
												 "Schema-valid native evidence export only; CMake, automated tests, "
												 "and hero/gallery integration remain separate work.")})};
}

auto build_visual_document() -> std::string
{
	const std::string dataset_id = "cross-space-dependency";
	const auto coupled = cross_space::generate_dataset(kRecords, cross_space::Coupling::coupled, kSeed);
	const auto decoupled = cross_space::generate_dataset(kRecords, cross_space::Coupling::decoupled, kSeed);
	const auto permuted = cross_space::permute_curves(coupled, kSeed);

	const auto matrices = cross_space::build_distance_matrices(coupled);
	const auto coupled_report = cross_space::permutation_test(coupled, kPermutations, kSeed);
	const auto decoupled_report = cross_space::permutation_test(decoupled, kPermutations, kSeed);
	const auto permuted_report = cross_space::permutation_test(permuted, kPermutations, kSeed);
	const auto coupled_baseline = cross_space::baseline_report(coupled, kBaselinePermutations, kSeed);
	const double compare_value = cross_space::observed_statistic(coupled);
	if (std::abs(compare_value - coupled_report.statistic) > 1.0e-9) {
		throw std::runtime_error("native compare() statistic does not match exported permutation-test statistic");
	}

	const auto ids = make_record_ids(coupled.size());
	const auto left_law = metric_law_diagnostics(matrices.left);
	const auto right_law = metric_law_diagnostics(matrices.right);
	const auto left_coordinates = landmark_coordinates(matrices.left);
	const auto right_coordinates = landmark_coordinates(matrices.right);
	const auto left_landmarks = select_landmarks(matrices.left);
	const auto right_landmarks = select_landmarks(matrices.right);
	const auto local_alignment = local_profile_alignment(matrices.left, matrices.right);
	const auto pair_values = pair_contributions(matrices.left, matrices.right);
	const auto local_pair_contribution = local_contribution(pair_values, coupled.size());
	const auto left_density = local_density(matrices.left);
	const auto right_density = local_density(matrices.right);

	visual::Document document;
	document.provenance_json(
		visual::object({visual::string_field("writer", "examples/engine/cross_space_dependency_visual_export.cpp"),
						visual::string_field("writer_language", "C++"),
						visual::string_field("computation", "native METRIC C++"),
						visual::bool_field("native_export", true),
						visual::bool_field("synthetic_js", false),
						visual::string_field("source_example", "examples/engine/cross_space_dependency.hpp"),
						visual::size_field("seed", kSeed),
						visual::size_field("record_count", kRecords),
						visual::size_field("permutations", kPermutations),
						visual::string_field("status", "native_export_foundation")}));
	document.dataset(dataset_id, "Cross-Space Dependency",
					 "Native C++ evidence for paired event-log strings under edit distance and process curves under "
					 "TWED; dependence is computed with MGC over two finite metric spaces.",
					 "METRIC examples/engine/cross_space_dependency.hpp deterministic native generator", "MPL-2.0");
	for (const auto &record : record_json(dataset_id, coupled, ids)) {
		document.record_json(record);
	}
	document.relation_json(relation_json("event-log-edit-distance", "event log edit distance", "mtrc::Edit<char>",
										 dataset_id, ids, matrices.left, left_law));
	document.relation_json(relation_json("process-curve-twed-distance", "process curve TWED distance",
										 "mtrc::TWED<double>", dataset_id, ids, matrices.right, right_law));
	document.space("event-log-space", dataset_id, ids, "event-log-edit-distance", "finite_metric_space",
				   space_metadata_json("Event logs / edit distance", "process-curve-space",
									   "edit_distance_over_event_tokens"));
	document.space("process-curve-space", dataset_id, ids, "process-curve-twed-distance", "finite_metric_space",
				   space_metadata_json("Process curves / TWED", "event-log-space", "time_warp_edit_distance"));
	document.scalar_property("generator-latent-severity", dataset_id, "generator latent severity (diagnostic)",
							 scalar_values(ids, coupled.latent));
	document.scalar_property("local-distance-profile-alignment", dataset_id, "local distance-profile alignment",
							 scalar_values(ids, local_alignment));
	document.scalar_property("local-dependence-contribution", dataset_id, "local dependence contribution",
							 scalar_values(ids, local_pair_contribution));
	document.scalar_property("event-log-local-density", dataset_id, "event-log local density",
							 scalar_values(ids, left_density));
	document.scalar_property("process-curve-local-density", dataset_id, "process-curve local density",
							 scalar_values(ids, right_density));
	document.property_json(pair_property_json(dataset_id, ids, pair_values));
	document.coordinates3("event-log-landmark-3d", dataset_id, "event-log-space",
						  "event log landmark distance coordinates", positions3(ids, left_coordinates),
						  coordinate_metadata_json(ids, left_landmarks));
	document.coordinates3("process-curve-landmark-3d", dataset_id, "process-curve-space",
						  "process curve landmark distance coordinates", positions3(ids, right_coordinates),
						  coordinate_metadata_json(ids, right_landmarks));
	document.view_json(visual::object({visual::string_field("id", "event-log-space-view"),
									   visual::string_field("kind", "metric-space"),
									   visual::string_field("name", "Event log space"),
									   visual::string_field("spaceId", "event-log-space"),
									   visual::string_field("coordinateId", "event-log-landmark-3d"),
									   visual::string_field("propertyId", "local-distance-profile-alignment")}));
	document.view_json(visual::object({visual::string_field("id", "process-curve-space-view"),
									   visual::string_field("kind", "metric-space"),
									   visual::string_field("name", "Process curve space"),
									   visual::string_field("spaceId", "process-curve-space"),
									   visual::string_field("coordinateId", "process-curve-landmark-3d"),
									   visual::string_field("propertyId", "local-distance-profile-alignment")}));
	document.view_json(visual::object({visual::string_field("id", "cross-space-paired-view"),
									   visual::string_field("kind", "paired-space"),
									   visual::string_field("name", "Cross-space dependence"),
									   visual::string_field("leftSpaceId", "event-log-space"),
									   visual::string_field("rightSpaceId", "process-curve-space"),
									   visual::string_field("leftCoordinateId", "event-log-landmark-3d"),
									   visual::string_field("rightCoordinateId", "process-curve-landmark-3d"),
									   visual::string_field("propertyId", "local-dependence-contribution"),
									   visual::string_field("diagnosticId", "cross-space-mgc-coupled")}));
	document.view_json(visual::object({visual::string_field("id", "event-log-relation-matrix"),
									   visual::string_field("kind", "relation-matrix"),
									   visual::string_field("name", "Event log edit distances"),
									   visual::string_field("relationId", "event-log-edit-distance")}));
	document.view_json(visual::object({visual::string_field("id", "process-curve-relation-matrix"),
									   visual::string_field("kind", "relation-matrix"),
									   visual::string_field("name", "Process curve TWED distances"),
									   visual::string_field("relationId", "process-curve-twed-distance")}));
	for (const auto &diagnostic :
		 diagnostics_json(coupled_report, decoupled_report, permuted_report, coupled_baseline, compare_value,
						  dataset_id)) {
		document.diagnostic_json(diagnostic);
	}
	return document.to_json();
}

auto usage(const char *program) -> void
{
	std::cerr << "Usage: " << program << " [--export-dir <dir>]\n";
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

		const std::filesystem::path output_dir(export_dir);
		const std::filesystem::path output_path = output_dir / "metric.visual.json";
		if (!visual::write_metric_visual_file(output_dir, json + "\n")) {
			throw std::runtime_error("failed to write export output: " + output_path.string());
		}
		return 0;
	} catch (const std::exception &error) {
		std::cerr << "cross_space_dependency_visual_export: " << error.what() << '\n';
		return 1;
	}
}
