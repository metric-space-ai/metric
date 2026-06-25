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
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cross_space_dependency.hpp"
#include "../../visual/cpp/mtrc_visual.hpp"

namespace {

namespace visual = mtrc::visual;

constexpr std::size_t kRecords = 512;
constexpr std::size_t kPermutations = 0;
constexpr std::size_t kBaselinePermutations = 999;
constexpr std::size_t kBridgeEvidenceLimit = 96;
constexpr std::size_t kTriangleCheckLimit = 2000000;
constexpr std::size_t kMgcSampleCount = 160;
constexpr std::size_t kMgcSampleIterations = 3;
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
	bool triangle_exhaustive{true};
	bool finite{true};
};

struct PairContribution {
	std::size_t row{};
	std::size_t column{};
	double value{};
};

struct BridgeEvidence {
	std::size_t index{};
	double local_contribution{};
	double profile_alignment{};
	double bridge_strength{};
};

auto sampled_mgc_options() -> mtrc::stats::correlate::mgc_options
{
	mtrc::stats::correlate::mgc_options options;
	options.sample_count = kMgcSampleCount;
	options.max_iterations = kMgcSampleIterations;
	return options;
}

auto sampled_dependence_report(const cross_space::Dataset &data) -> cross_space::DependenceReport
{
	auto log_space = mtrc::make_space(data.logs, cross_space::LogMetric{});
	auto curve_space = mtrc::make_space(data.curves, cross_space::curve_metric());
	const auto result = mtrc::stats::correlate::mgc_estimate(log_space, curve_space, sampled_mgc_options());

	cross_space::DependenceReport report;
	report.record_count = data.size();
	report.permutations = 0;
	report.statistic = result.value;
	report.p_value = 1.0;
	return report;
}

auto baseline_report_sampled(const cross_space::Dataset &data, std::size_t permutations, std::uint64_t seed)
	-> cross_space::BaselineReport
{
	cross_space::BaselineReport report;
	report.permutations = permutations;

	std::vector<double> log_scalar;
	std::vector<double> curve_scalar;
	log_scalar.reserve(data.size());
	curve_scalar.reserve(data.size());
	for (std::size_t index = 0; index < data.size(); ++index) {
		log_scalar.push_back(cross_space::log_length(data.logs[index]));
		curve_scalar.push_back(cross_space::curve_mean(data.curves[index]));
	}

	report.pearson_r = cross_space::pearson(log_scalar, curve_scalar);

	std::vector<double> shuffled = curve_scalar;
	auto rng = cross_space::detail::stream(seed, 11);
	std::size_t at_least_as_extreme = 0;
	const double observed_abs = std::abs(report.pearson_r);
	for (std::size_t draw = 0; draw < permutations; ++draw) {
		cross_space::detail::portable_shuffle(shuffled, rng);
		if (std::abs(cross_space::pearson(log_scalar, shuffled)) >= observed_abs) {
			++at_least_as_extreme;
		}
	}
	report.pearson_p_value =
		(1.0 + static_cast<double>(at_least_as_extreme)) / (1.0 + static_cast<double>(permutations));

	std::size_t log_width = 0;
	std::size_t curve_width = 0;
	for (const auto &log : data.logs) {
		log_width = std::max(log_width, log.size());
	}
	for (const auto &curve : data.curves) {
		curve_width = std::max(curve_width, curve.size());
	}
	std::vector<std::vector<double>> log_vectors;
	std::vector<std::vector<double>> curve_vectors;
	log_vectors.reserve(data.size());
	curve_vectors.reserve(data.size());
	for (std::size_t index = 0; index < data.size(); ++index) {
		log_vectors.push_back(cross_space::encode_log_vector(data.logs[index], log_width));
		curve_vectors.push_back(cross_space::encode_curve_vector(data.curves[index], curve_width));
	}
	auto log_vector_space = mtrc::make_space(log_vectors, mtrc::Euclidean<double>{});
	auto curve_vector_space = mtrc::make_space(curve_vectors, mtrc::Euclidean<double>{});
	report.vectorized_mgc =
		mtrc::stats::correlate::mgc_estimate(log_vector_space, curve_vector_space, sampled_mgc_options()).value;

	return report;
}

auto record_id(std::size_t index) -> std::string
{
	std::ostringstream out;
	out << "obs-" << std::setw(3) << std::setfill('0') << index;
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

	const std::size_t exhaustive_triplets = count * count * count;
	if (exhaustive_triplets <= kTriangleCheckLimit) {
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
	} else {
		diagnostics.triangle_exhaustive = false;
		diagnostics.triangle_triplets = kTriangleCheckLimit;
		for (std::size_t sample = 0; sample < kTriangleCheckLimit; ++sample) {
			const std::size_t i = (sample * 15485863ULL + 11ULL) % count;
			const std::size_t j = (sample * 32452843ULL + 23ULL) % count;
			const std::size_t k = (sample * 49979687ULL + 37ULL) % count;
			const double violation =
				matrix_value(matrix, i, k) - (matrix_value(matrix, i, j) + matrix_value(matrix, j, k));
			diagnostics.triangle_max_violation = std::max(diagnostics.triangle_max_violation, violation);
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

auto selected_bridge_evidence(const std::vector<double> &local_contribution,
							  const std::vector<double> &profile_alignment) -> std::vector<BridgeEvidence>
{
	std::vector<BridgeEvidence> evidence;
	evidence.reserve(local_contribution.size());
	for (std::size_t index = 0; index < local_contribution.size(); ++index) {
		const double contribution = local_contribution[index];
		const double alignment = index < profile_alignment.size() ? profile_alignment[index] : 0.0;
		const double strength = std::abs(contribution) * (0.5 + 0.5 * std::max(0.0, alignment));
		evidence.push_back(BridgeEvidence{index, contribution, alignment, strength});
	}
	std::sort(evidence.begin(), evidence.end(), [](const auto &left, const auto &right) {
		if (left.bridge_strength == right.bridge_strength) {
			return left.index < right.index;
		}
		return left.bridge_strength > right.bridge_strength;
	});
	if (evidence.size() > kBridgeEvidenceLimit) {
		evidence.resize(kBridgeEvidenceLimit);
	}
	std::sort(evidence.begin(), evidence.end(), [](const auto &left, const auto &right) {
		return left.index < right.index;
	});
	return evidence;
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
													  visual::string_field("triangle",
																		   diagnostics.triangle_exhaustive
																			   ? "checked_exhaustive"
																			   : "checked_deterministic_sample"),
													  visual::bool_field("finite", diagnostics.finite),
													  visual::bool_field("triangle_exhaustive",
																		 diagnostics.triangle_exhaustive),
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

auto bridge_relation_json(const std::string &dataset_id, const std::vector<std::string> &record_ids,
						  const std::vector<BridgeEvidence> &bridges) -> std::string
{
	std::vector<std::string> values;
	values.reserve(bridges.size());
	for (const auto &bridge : bridges) {
		values.push_back(visual::object({visual::string_field("row_id", record_ids[bridge.index]),
										 visual::string_field("column_id", record_ids[bridge.index]),
										 visual::number_field("value", bridge.bridge_strength)}));
	}
	return visual::object(
		{visual::string_field("id", "cross-space-dependence-bridge-relation"),
		 visual::string_field("dataset_id", dataset_id),
		 visual::string_field("name", "high-contribution paired-space bridge strength"),
		 visual::string_field("relation_type", "custom"),
		 visual::string_field("value_type", "scalar"),
		 visual::string_array_field("record_ids", record_ids),
		 visual::string_field("storage", "sparse_edge_list"),
		 visual::field("values", visual::array_of(values)),
		 visual::field("metadata",
					   visual::object({visual::string_field("computed_by", "METRIC native C++ exporter"),
									   visual::string_field("bridge_selection",
															"top absolute local-dependence contribution weighted by positive distance-profile alignment"),
									   visual::string_field("source_property",
															"local-dependence-contribution"),
									   visual::string_field("alignment_property",
															"local-distance-profile-alignment"),
									   visual::size_field("selected_bridge_count", bridges.size()),
									   visual::size_field("candidate_pair_count", record_ids.size())}))});
}

auto bridge_graph_json(const std::string &dataset_id, const std::vector<std::string> &record_ids,
					   const std::vector<BridgeEvidence> &bridges) -> std::string
{
	std::vector<std::string> edges;
	edges.reserve(bridges.size());
	for (const auto &bridge : bridges) {
		edges.push_back(visual::object(
			{visual::string_field("source_id", record_ids[bridge.index]),
			 visual::string_field("target_id", record_ids[bridge.index]),
			 visual::number_field("value", bridge.bridge_strength),
			 visual::number_field("weight", bridge.bridge_strength),
			 visual::number_field("local_dependence_contribution", bridge.local_contribution),
			 visual::number_field("local_distance_profile_alignment", bridge.profile_alignment),
			 visual::string_field("source_space_id", "event-log-space"),
			 visual::string_field("target_space_id", "process-curve-space"),
			 visual::string_field("source_coordinate_id", "event-log-landmark-3d"),
			 visual::string_field("target_coordinate_id", "process-curve-landmark-3d")}));
	}
	return visual::object(
		{visual::string_field("id", "cross-space-dependence-bridges"),
		 visual::string_field("dataset_id", dataset_id),
		 visual::string_array_field("node_record_ids", record_ids),
		 visual::string_field("edge_relation_id", "cross-space-dependence-bridge-relation"),
		 visual::string_field("graph_type", "paired-space dependence bridge"),
		 visual::field("edges", visual::array_of(edges)),
		 visual::field("metadata",
					   visual::object({visual::string_field("native_evidence", "paired-space bridge geometry"),
									   visual::string_field("source_space_id", "event-log-space"),
									   visual::string_field("target_space_id", "process-curve-space"),
									   visual::string_field("source_coordinate_id", "event-log-landmark-3d"),
									   visual::string_field("target_coordinate_id", "process-curve-landmark-3d"),
									   visual::string_field("selection",
															"top native local-dependence contributors"),
									   visual::size_field("edge_count", bridges.size()),
									   visual::size_field("candidate_pair_count", record_ids.size())}))});
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
	std::vector<visual::Field> fields{visual::number_field("statistic", report.statistic),
									  visual::number_field("null_mean", report.null_mean),
									  visual::number_field("null_sd", report.null_sd),
									  visual::number_field("standardized", report.standardized),
									  visual::number_field("p_value", report.p_value),
									  visual::size_field("permutations", report.permutations),
									  visual::size_field("record_count", report.record_count)};
	if (report.permutations == 0) {
		fields.push_back(visual::string_field("decision", "not_tested"));
		fields.push_back(visual::string_field("permutation_status",
											  "omitted_cost_control_exact_mgc_permutations_not_tractable_at_export_scale"));
	} else {
		fields.push_back(visual::string_field("decision", report.p_value <= kAlpha ? "dependent" : "independent"));
		fields.push_back(visual::string_field("permutation_status", "computed"));
	}
	return visual::object(fields);
}

auto diagnostics_json(const cross_space::DependenceReport &coupled, const cross_space::DependenceReport &decoupled,
					  const cross_space::DependenceReport &permuted,
					  const cross_space::BaselineReport &coupled_baseline, double compare_value,
					  const std::string &dataset_id, const std::vector<BridgeEvidence> &bridges) -> std::vector<std::string>
{
	auto diagnostics = std::vector<std::string>{
		visual::object({visual::string_field("id", "cross-space-mgc-coupled"),
						visual::string_field("kind", "cross_space_dependence"),
						visual::string_field("dataset_id", dataset_id),
						visual::field("space_ids",
									  visual::string_array({"event-log-space", "process-curve-space"})),
						visual::field("relation_ids",
									  visual::string_array({"event-log-edit-distance",
															"process-curve-twed-distance"})),
						visual::string_field("algorithm",
											 "METRIC native stats::correlate::mgc_estimate over paired spaces; exact full-grid MGC permutations omitted by scale cost control"),
						visual::number_field("compare_statistic", compare_value),
						visual::size_field("mgc_sample_count", kMgcSampleCount),
						visual::size_field("mgc_sample_iterations", kMgcSampleIterations),
						visual::bool_field("exact_full_grid_mgc", false),
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
		visual::object({visual::string_field("id", "cross-space-bridge-evidence"),
						visual::string_field("kind", "paired_space_bridge_evidence"),
						visual::string_field("dataset_id", dataset_id),
						visual::string_field("graph_id", "cross-space-dependence-bridges"),
						visual::string_field("relation_id", "cross-space-dependence-bridge-relation"),
						visual::string_field("selection", "top native local-dependence contributors"),
						visual::size_field("selected_bridge_count", bridges.size()),
						visual::size_field("candidate_pair_count", kRecords)}),
		visual::object({visual::string_field("id", "native-export-foundation"),
						visual::string_field("kind", "integration_status"),
						visual::string_field("dataset_id", dataset_id),
						visual::string_field("status", "native_cpp_metric_visual_export_foundation"),
						visual::bool_field("public_hero_ready", false),
						visual::string_field("note",
											 "Schema-valid native evidence export only; screenshot review remains separate work.")})};
	return diagnostics;
}

auto build_visual_document() -> std::string
{
	const std::string dataset_id = "cross-space-dependency";
	const auto coupled = cross_space::generate_dataset(kRecords, cross_space::Coupling::coupled, kSeed);
	const auto decoupled = cross_space::generate_dataset(kRecords, cross_space::Coupling::decoupled, kSeed);
	const auto permuted = cross_space::permute_curves(coupled, kSeed);

	const auto matrices = cross_space::build_distance_matrices(coupled);
	const auto coupled_report = sampled_dependence_report(coupled);
	const auto decoupled_report = sampled_dependence_report(decoupled);
	const auto permuted_report = sampled_dependence_report(permuted);
	const auto coupled_baseline = baseline_report_sampled(coupled, kBaselinePermutations, kSeed);
	const double compare_value = coupled_report.statistic;

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
	const auto bridge_evidence = selected_bridge_evidence(local_pair_contribution, local_alignment);

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
	document.relation_json(bridge_relation_json(dataset_id, ids, bridge_evidence));
	document.graph_json(bridge_graph_json(dataset_id, ids, bridge_evidence));
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
									   visual::string_field("bridgeGraphId", "cross-space-dependence-bridges"),
									   visual::string_field("bridgeRelationId",
															"cross-space-dependence-bridge-relation"),
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
						  dataset_id, bridge_evidence)) {
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
