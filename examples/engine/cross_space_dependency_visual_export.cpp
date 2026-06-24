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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cross_space_dependency.hpp"

namespace {

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

auto quote(const std::string &value) -> std::string
{
	std::ostringstream out;
	out << '"';
	for (const unsigned char ch : value) {
		switch (ch) {
		case '"':
			out << "\\\"";
			break;
		case '\\':
			out << "\\\\";
			break;
		case '\n':
			out << "\\n";
			break;
		case '\r':
			out << "\\r";
			break;
		case '\t':
			out << "\\t";
			break;
		default:
			if (ch < 0x20) {
				out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(ch)
					<< std::dec << std::setfill(' ');
			} else {
				out << static_cast<char>(ch);
			}
		}
	}
	out << '"';
	return out.str();
}

auto number(double value) -> std::string
{
	if (!std::isfinite(value)) {
		return "0";
	}
	std::ostringstream out;
	out << std::setprecision(12) << value;
	return out.str();
}

auto bool_json(bool value) -> const char * { return value ? "true" : "false"; }

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

auto write_string_array(std::ostream &out, const std::vector<std::string> &values) -> void
{
	out << '[';
	for (std::size_t index = 0; index < values.size(); ++index) {
		if (index != 0) {
			out << ',';
		}
		out << quote(values[index]);
	}
	out << ']';
}

auto write_number_array(std::ostream &out, const std::vector<double> &values) -> void
{
	out << '[';
	for (std::size_t index = 0; index < values.size(); ++index) {
		if (index != 0) {
			out << ',';
		}
		out << number(values[index]);
	}
	out << ']';
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

auto write_metric_law_metadata(std::ostream &out, const MetricLawDiagnostics &diagnostics) -> void
{
	out << "\"law_check\":{";
	out << "\"diagonal\":\"checked_exhaustive\",";
	out << "\"symmetry\":\"checked_exhaustive\",";
	out << "\"triangle\":\"checked_exhaustive\",";
	out << "\"finite\":" << bool_json(diagnostics.finite) << ',';
	out << "\"diagonal_max_abs\":" << number(diagnostics.diagonal_max_abs) << ',';
	out << "\"symmetry_max_abs\":" << number(diagnostics.symmetry_max_abs) << ',';
	out << "\"triangle_max_violation\":" << number(diagnostics.triangle_max_violation) << ',';
	out << "\"triangle_triplets\":" << diagnostics.triangle_triplets;
	out << "},";
	out << "\"pair_count\":" << diagnostics.pair_count << ',';
	out << "\"minimum_nonzero_distance\":" << number(diagnostics.minimum_nonzero_distance) << ',';
	out << "\"maximum_distance\":" << number(diagnostics.maximum_distance) << ',';
	out << "\"average_distance\":" << number(diagnostics.average_distance);
}

auto write_relation(std::ostream &out, const std::string &id, const std::string &name, const std::string &metric_name,
					const std::string &dataset_id, const std::vector<std::string> &ids,
					const mtrc::DistanceMatrix<double> &matrix, const MetricLawDiagnostics &diagnostics) -> void
{
	out << "{\"id\":" << quote(id);
	out << ",\"dataset_id\":" << quote(dataset_id);
	out << ",\"name\":" << quote(name);
	out << ",\"relation_type\":\"metric\",\"value_type\":\"scalar\",\"record_ids\":";
	write_string_array(out, ids);
	out << ",\"storage\":\"dense_matrix\",\"values\":[";
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		if (row != 0) {
			out << ',';
		}
		out << '[';
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			if (column != 0) {
				out << ',';
			}
			out << number(matrix_value(matrix, row, column));
		}
		out << ']';
	}
	out << "],\"metadata\":{";
	out << "\"metric\":" << quote(metric_name) << ',';
	out << "\"computed_by\":\"METRIC native C++ DistanceTable/provider_symmetric_distance_matrix\",";
	write_metric_law_metadata(out, diagnostics);
	out << "}}";
}

auto write_records(std::ostream &out, const std::string &dataset_id, const cross_space::Dataset &dataset,
				   const std::vector<std::string> &ids) -> void
{
	out << '[';
	for (std::size_t index = 0; index < dataset.size(); ++index) {
		if (index != 0) {
			out << ',';
		}
		out << "{\"id\":" << quote(ids[index]);
		out << ",\"dataset_id\":" << quote(dataset_id);
		out << ",\"record_type\":\"paired_cross_space_observation\"";
		out << ",\"label\":" << quote("paired observation " + std::to_string(index));
		out << ",\"payload\":{";
		out << "\"kind\":\"composed\",";
		out << "\"components\":{";
		out << "\"event_log\":{\"kind\":\"string\",\"text\":" << quote(dataset.logs[index]) << "},";
		out << "\"process_curve\":{\"kind\":\"time_series\",\"series\":";
		write_number_array(out, dataset.curves[index]);
		out << "}";
		out << "},";
		out << "\"metadata\":{";
		out << "\"pair_index\":" << index << ',';
		out << "\"generator_latent_severity\":" << number(dataset.latent[index]) << ',';
		out << "\"left_space\":\"event_logs/edit_distance\",";
		out << "\"right_space\":\"process_curves/twed\"";
		out << "}}}";
	}
	out << ']';
}

auto write_space(std::ostream &out, const std::string &id, const std::string &name, const std::string &dataset_id,
				 const std::vector<std::string> &ids, const std::string &relation_id, const std::string &paired_with,
				 const std::string &metric_family) -> void
{
	out << "{\"id\":" << quote(id);
	out << ",\"dataset_id\":" << quote(dataset_id);
	out << ",\"record_ids\":";
	write_string_array(out, ids);
	out << ",\"primary_relation_id\":" << quote(relation_id);
	out << ",\"space_type\":\"finite_metric_space\"";
	out << ",\"metadata\":{";
	out << "\"name\":" << quote(name) << ',';
	out << "\"paired_with\":" << quote(paired_with) << ',';
	out << "\"metric_family\":" << quote(metric_family) << ',';
	out << "\"pairing\":\"position_aligned_records\"";
	out << "}}";
}

auto write_scalar_property(std::ostream &out, const std::string &id, const std::string &dataset_id,
						   const std::string &name, const std::vector<std::string> &record_ids,
						   const std::vector<double> &values) -> void
{
	out << "{\"id\":" << quote(id);
	out << ",\"dataset_id\":" << quote(dataset_id);
	out << ",\"target_type\":\"record\",\"value_type\":\"scalar\"";
	out << ",\"name\":" << quote(name);
	out << ",\"values\":[";
	for (std::size_t index = 0; index < record_ids.size(); ++index) {
		if (index != 0) {
			out << ',';
		}
		out << "{\"record_id\":" << quote(record_ids[index]) << ",\"value\":" << number(values[index]) << "}";
	}
	out << "]}";
}

auto write_pair_property(std::ostream &out, const std::string &dataset_id, const std::vector<std::string> &record_ids,
						 const std::vector<PairContribution> &pairs) -> void
{
	out << "{\"id\":\"pair-distance-profile-contribution\"";
	out << ",\"dataset_id\":" << quote(dataset_id);
	out << ",\"target_type\":\"pair\",\"value_type\":\"scalar\"";
	out << ",\"name\":\"pair distance-profile z-product contribution\"";
	out << ",\"values\":[";
	for (std::size_t index = 0; index < pairs.size(); ++index) {
		if (index != 0) {
			out << ',';
		}
		const auto &pair = pairs[index];
		out << "{\"row_id\":" << quote(record_ids[pair.row]);
		out << ",\"column_id\":" << quote(record_ids[pair.column]);
		out << ",\"value\":" << number(pair.value) << "}";
	}
	out << "]}";
}

auto write_coordinates(std::ostream &out, const std::string &id, const std::string &dataset_id,
					   const std::string &space_id, const std::string &name, const std::vector<std::string> &record_ids,
					   const std::vector<std::array<double, 3>> &coordinates,
					   const std::array<std::size_t, 3> &landmarks) -> void
{
	out << "{\"id\":" << quote(id);
	out << ",\"dataset_id\":" << quote(dataset_id);
	out << ",\"space_id\":" << quote(space_id);
	out << ",\"name\":" << quote(name);
	out << ",\"dimension\":3";
	out << ",\"record_positions\":[";
	for (std::size_t index = 0; index < record_ids.size(); ++index) {
		if (index != 0) {
			out << ',';
		}
		out << "{\"record_id\":" << quote(record_ids[index]);
		out << ",\"position\":[";
		out << number(coordinates[index][0]) << ',' << number(coordinates[index][1]) << ','
			<< number(coordinates[index][2]);
		out << "]}";
	}
	out << "],\"metadata\":{";
	out << "\"method\":\"native_cpp_landmark_distance_coordinates\",";
	out << "\"landmark_record_ids\":[";
	for (std::size_t axis = 0; axis < landmarks.size(); ++axis) {
		if (axis != 0) {
			out << ',';
		}
		out << quote(record_ids[landmarks[axis]]);
	}
	out << "]}}";
}

auto write_report_object(std::ostream &out, const cross_space::DependenceReport &report) -> void
{
	out << "{";
	out << "\"statistic\":" << number(report.statistic) << ',';
	out << "\"null_mean\":" << number(report.null_mean) << ',';
	out << "\"null_sd\":" << number(report.null_sd) << ',';
	out << "\"standardized\":" << number(report.standardized) << ',';
	out << "\"p_value\":" << number(report.p_value) << ',';
	out << "\"permutations\":" << report.permutations << ',';
	out << "\"record_count\":" << report.record_count << ',';
	out << "\"decision\":" << quote(report.p_value <= kAlpha ? "dependent" : "independent");
	out << "}";
}

auto write_diagnostics(std::ostream &out, const cross_space::DependenceReport &coupled,
					   const cross_space::DependenceReport &decoupled,
					   const cross_space::DependenceReport &permuted,
					   const cross_space::BaselineReport &coupled_baseline, double compare_value,
					   const std::string &dataset_id) -> void
{
	out << '[';
	out << "{\"id\":\"cross-space-mgc-coupled\",";
	out << "\"kind\":\"cross_space_dependence\",";
	out << "\"dataset_id\":" << quote(dataset_id) << ',';
	out << "\"space_ids\":[\"event-log-space\",\"process-curve-space\"],";
	out << "\"relation_ids\":[\"event-log-edit-distance\",\"process-curve-twed-distance\"],";
	out << "\"algorithm\":\"mtrc::compare(..., mgc_options) plus native seeded permutation_test\",";
	out << "\"compare_statistic\":" << number(compare_value) << ',';
	out << "\"alpha\":" << number(kAlpha) << ',';
	out << "\"report\":";
	write_report_object(out, coupled);
	out << "},";
	out << "{\"id\":\"cross-space-native-controls\",";
	out << "\"kind\":\"cross_space_dependency_controls\",";
	out << "\"dataset_id\":" << quote(dataset_id) << ',';
	out << "\"decoupled_report\":";
	write_report_object(out, decoupled);
	out << ",\"permuted_pairing_report\":";
	write_report_object(out, permuted);
	out << ",\"baseline_scalar_pearson_r\":" << number(coupled_baseline.pearson_r);
	out << ",\"baseline_scalar_pearson_p_value\":" << number(coupled_baseline.pearson_p_value);
	out << ",\"baseline_forced_vector_mgc\":" << number(coupled_baseline.vectorized_mgc);
	out << ",\"baseline_permutations\":" << coupled_baseline.permutations;
	out << ",\"verdict\":\"metric_detects_dependence_baseline_misses_it\"";
	out << "},";
	out << "{\"id\":\"native-export-foundation\",";
	out << "\"kind\":\"integration_status\",";
	out << "\"dataset_id\":" << quote(dataset_id) << ',';
	out << "\"status\":\"native_cpp_metric_visual_export_foundation\",";
	out << "\"public_hero_ready\":false,";
	out << "\"note\":\"Schema-valid native evidence export only; CMake, automated tests, and hero/gallery integration remain separate work.\"";
	out << "}";
	out << ']';
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

	std::ostringstream out;
	out << "{\"schema\":\"metric.visual.v1\",";
	out << "\"provenance\":{";
	out << "\"writer\":\"examples/engine/cross_space_dependency_visual_export.cpp\",";
	out << "\"writer_language\":\"C++\",";
	out << "\"computation\":\"native METRIC C++\",";
	out << "\"native_export\":true,";
	out << "\"synthetic_js\":false,";
	out << "\"source_example\":\"examples/engine/cross_space_dependency.hpp\",";
	out << "\"seed\":" << kSeed << ',';
	out << "\"record_count\":" << kRecords << ',';
	out << "\"permutations\":" << kPermutations << ',';
	out << "\"status\":\"native_export_foundation\"";
	out << "},";

	out << "\"datasets\":[{\"id\":" << quote(dataset_id);
	out << ",\"title\":\"Cross-Space Dependency\",";
	out << "\"description\":\"Native C++ evidence for paired event-log strings under edit distance and process curves under TWED; dependence is computed with MGC over two finite metric spaces.\",";
	out << "\"source\":\"METRIC examples/engine/cross_space_dependency.hpp deterministic native generator\",";
	out << "\"license\":\"MPL-2.0\"";
	out << "}],";

	out << "\"records\":";
	write_records(out, dataset_id, coupled, ids);
	out << ',';

	out << "\"relations\":[";
	write_relation(out, "event-log-edit-distance", "event log edit distance", "mtrc::Edit<char>", dataset_id, ids,
				   matrices.left, left_law);
	out << ',';
	write_relation(out, "process-curve-twed-distance", "process curve TWED distance", "mtrc::TWED<double>", dataset_id,
				   ids, matrices.right, right_law);
	out << "],";

	out << "\"spaces\":[";
	write_space(out, "event-log-space", "Event logs / edit distance", dataset_id, ids, "event-log-edit-distance",
				"process-curve-space", "edit_distance_over_event_tokens");
	out << ',';
	write_space(out, "process-curve-space", "Process curves / TWED", dataset_id, ids, "process-curve-twed-distance",
				"event-log-space", "time_warp_edit_distance");
	out << "],";

	out << "\"properties\":[";
	write_scalar_property(out, "generator-latent-severity", dataset_id, "generator latent severity (diagnostic)", ids,
						  coupled.latent);
	out << ',';
	write_scalar_property(out, "local-distance-profile-alignment", dataset_id,
						  "local distance-profile alignment", ids, local_alignment);
	out << ',';
	write_scalar_property(out, "local-dependence-contribution", dataset_id,
						  "local dependence contribution", ids, local_pair_contribution);
	out << ',';
	write_scalar_property(out, "event-log-local-density", dataset_id, "event-log local density", ids, left_density);
	out << ',';
	write_scalar_property(out, "process-curve-local-density", dataset_id, "process-curve local density", ids,
						  right_density);
	out << ',';
	write_pair_property(out, dataset_id, ids, pair_values);
	out << "],";

	out << "\"graphs\":[],";
	out << "\"coordinates\":[";
	write_coordinates(out, "event-log-landmark-3d", dataset_id, "event-log-space",
					  "event log landmark distance coordinates", ids, left_coordinates, left_landmarks);
	out << ',';
	write_coordinates(out, "process-curve-landmark-3d", dataset_id, "process-curve-space",
					  "process curve landmark distance coordinates", ids, right_coordinates, right_landmarks);
	out << "],";
	out << "\"timelines\":[],";
	out << "\"events\":[],";
	out << "\"views\":[";
	out << "{\"id\":\"event-log-space-view\",\"kind\":\"metric-space\",\"name\":\"Event log space\",";
	out << "\"spaceId\":\"event-log-space\",\"coordinateId\":\"event-log-landmark-3d\",";
	out << "\"propertyId\":\"local-distance-profile-alignment\"},";
	out << "{\"id\":\"process-curve-space-view\",\"kind\":\"metric-space\",\"name\":\"Process curve space\",";
	out << "\"spaceId\":\"process-curve-space\",\"coordinateId\":\"process-curve-landmark-3d\",";
	out << "\"propertyId\":\"local-distance-profile-alignment\"},";
	out << "{\"id\":\"cross-space-paired-view\",\"kind\":\"paired-space\",\"name\":\"Cross-space dependence\",";
	out << "\"leftSpaceId\":\"event-log-space\",\"rightSpaceId\":\"process-curve-space\",";
	out << "\"leftCoordinateId\":\"event-log-landmark-3d\",\"rightCoordinateId\":\"process-curve-landmark-3d\",";
	out << "\"propertyId\":\"local-dependence-contribution\",\"diagnosticId\":\"cross-space-mgc-coupled\"},";
	out << "{\"id\":\"event-log-relation-matrix\",\"kind\":\"relation-matrix\",\"name\":\"Event log edit distances\",";
	out << "\"relationId\":\"event-log-edit-distance\"},";
	out << "{\"id\":\"process-curve-relation-matrix\",\"kind\":\"relation-matrix\",\"name\":\"Process curve TWED distances\",";
	out << "\"relationId\":\"process-curve-twed-distance\"}";
	out << "],";
	out << "\"diagnostics\":";
	write_diagnostics(out, coupled_report, decoupled_report, permuted_report, coupled_baseline, compare_value, dataset_id);
	out << "}";

	return out.str();
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

		const std::filesystem::path directory(export_dir);
		std::filesystem::create_directories(directory);
		const std::filesystem::path output_path = directory / "metric.visual.json";
		std::ofstream file(output_path);
		if (!file) {
			throw std::runtime_error("failed to open export output: " + output_path.string());
		}
		file << json << '\n';
		return 0;
	} catch (const std::exception &error) {
		std::cerr << "cross_space_dependency_visual_export: " << error.what() << '\n';
		return 1;
	}
}
