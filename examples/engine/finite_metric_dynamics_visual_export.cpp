// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Native metric.visual.v1 exporter for the Finite Metric Dynamics example.
//
// This file is intentionally self-contained: it computes the same native C++
// dynamics evidence as finite_metric_dynamics.cpp and serializes it directly to
// metric.visual.v1 JSON. No JavaScript or Python participates in the evidence
// computation path.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <metric/engine.hpp>
#include <metric/metric/catalog/vector/Standards.hpp>

#include "../../visual/cpp/mtrc_visual.hpp"
#include "finite_metric_dynamics_fixture.hpp"

namespace {

namespace visual = mtrc::visual;

using Signal = std::vector<std::vector<double>>;
using Matrix = std::vector<std::vector<double>>;

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

auto node_id(std::size_t i) -> std::string
{
	std::ostringstream out;
	out << "node-";
	if (i < 10) {
		out << "0";
	}
	out << i;
	return out.str();
}

auto step_suffix(std::size_t i) -> std::string
{
	std::ostringstream out;
	if (i < 10) {
		out << "0";
	}
	out << i;
	return out.str();
}

auto step_id(const std::string &prefix, std::size_t i) -> std::string { return prefix + "-" + step_suffix(i); }

auto coordinate_id(const std::string &prefix, std::size_t i) -> std::string
{
	return "coord-" + prefix + "-" + step_suffix(i);
}

auto signal_of_records(const std::vector<std::vector<double>> &records) -> Signal { return records; }

template <typename Space> auto signal_of_space(const Space &space) -> Signal
{
	Signal signal;
	signal.reserve(space.size());
	for (std::size_t i = 0; i < space.size(); ++i) {
		const auto &record = space.record(space.id(i));
		signal.emplace_back(record.begin(), record.end());
	}
	return signal;
}

auto mse(const Signal &lhs, const Signal &rhs) -> double
{
	double accum = 0.0;
	std::size_t terms = 0;
	for (std::size_t i = 0; i < lhs.size(); ++i) {
		for (std::size_t c = 0; c < lhs[i].size(); ++c) {
			const double delta = lhs[i][c] - rhs[i][c];
			accum += delta * delta;
			++terms;
		}
	}
	return terms == 0 ? 0.0 : accum / static_cast<double>(terms);
}

auto node_mse(const std::vector<double> &lhs, const std::vector<double> &rhs) -> double
{
	double accum = 0.0;
	for (std::size_t c = 0; c < lhs.size(); ++c) {
		const double delta = lhs[c] - rhs[c];
		accum += delta * delta;
	}
	return lhs.empty() ? 0.0 : accum / static_cast<double>(lhs.size());
}

auto signals_equal(const Signal &lhs, const Signal &rhs) -> bool
{
	if (lhs.size() != rhs.size()) {
		return false;
	}
	for (std::size_t i = 0; i < lhs.size(); ++i) {
		if (lhs[i].size() != rhs[i].size()) {
			return false;
		}
		for (std::size_t c = 0; c < lhs[i].size(); ++c) {
			if (lhs[i][c] != rhs[i][c]) {
				return false;
			}
		}
	}
	return true;
}

auto entropy(const std::vector<double> &distribution) -> double
{
	double value = 0.0;
	for (const double p : distribution) {
		if (p > 0.0) {
			value -= p * std::log(p);
		}
	}
	return value;
}

template <typename Space> auto native_distance_matrix(const Space &space) -> Matrix
{
	const std::size_t n = space.size();
	Matrix distances(n, std::vector<double>(n, 0.0));
	for (std::size_t i = 0; i < n; ++i) {
		const auto id_i = space.id(i);
		for (std::size_t j = i + 1; j < n; ++j) {
			const auto id_j = space.id(j);
			const double value = static_cast<double>(space.distance(id_i, id_j));
			distances[i][j] = value;
			distances[j][i] = value;
		}
	}
	return distances;
}

auto dense_matrix_json(const Matrix &matrix) -> std::string
{
	std::vector<std::string> rows;
	rows.reserve(matrix.size());
	for (const auto &row : matrix) {
		rows.push_back(visual::number_array(row));
	}
	return visual::array_of(rows);
}

auto integer_array_json(const std::vector<std::size_t> &values) -> std::string
{
	std::vector<std::string> items;
	items.reserve(values.size());
	for (const auto value : values) {
		items.push_back(std::to_string(value));
	}
	return visual::array_of(items);
}

auto scalar_record_property(const std::string &id, const std::string &dataset_id, const std::string &name,
							const std::vector<std::string> &record_ids, const std::vector<double> &values,
							const std::string &description) -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(record_ids.size());
	for (std::size_t i = 0; i < record_ids.size(); ++i) {
		entries.push_back(json_object({{"record_id", visual::quote(record_ids[i])}, {"value", visual::num(values[i])}}));
	}
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(dataset_id)},
						{"name", visual::quote(name)},
						{"target_type", visual::quote("record")},
						{"value_type", visual::quote("scalar")},
						{"values", visual::array_of(entries)},
						{"metadata", json_object({{"description", visual::quote(description)}})}});
}

auto scalar_record_property_from_size_t(const std::string &id, const std::string &dataset_id, const std::string &name,
										const std::vector<std::string> &record_ids,
										const std::vector<std::size_t> &values,
										const std::string &description) -> std::string
{
	std::vector<double> numeric;
	numeric.reserve(values.size());
	for (const auto value : values) {
		numeric.push_back(static_cast<double>(value));
	}
	return scalar_record_property(id, dataset_id, name, record_ids, numeric, description);
}

auto timeline_step_property(const std::string &id, const std::string &dataset_id, const std::string &name,
							const std::string &timeline_id, const std::vector<double> &values,
							const std::string &description) -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(values.size());
	for (std::size_t i = 0; i < values.size(); ++i) {
		entries.push_back(json_object({{"timeline_id", visual::quote(timeline_id)},
									   {"step_id", visual::quote(step_id(timeline_id, i))},
									   {"index", json_size(i)},
									   {"value", visual::num(values[i])}}));
	}
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(dataset_id)},
						{"name", visual::quote(name)},
						{"target_type", visual::quote("timeline_step")},
						{"value_type", visual::quote("scalar")},
						{"values", visual::array_of(entries)},
						{"metadata", json_object({{"description", visual::quote(description)}})}});
}

auto timeline_step_property_from_signal(const std::string &id, const std::string &dataset_id, const std::string &name,
										const std::string &timeline_id, const std::vector<Signal> &frames,
										const Signal &clean, const std::string &description) -> std::string
{
	std::vector<double> values;
	values.reserve(frames.size());
	for (const auto &frame : frames) {
		values.push_back(mse(frame, clean));
	}
	return timeline_step_property(id, dataset_id, name, timeline_id, values, description);
}

auto coordinate_json(const std::string &id, const std::string &dataset_id, const std::string &space_id,
					 const std::string &name, const Signal &frame, const std::vector<double> &stationary,
					 double z_scale, const std::string &source_operator, std::size_t step) -> std::string
{
	std::vector<std::string> positions;
	positions.reserve(frame.size());
	for (std::size_t i = 0; i < frame.size(); ++i) {
		std::vector<double> position = {frame[i][0], frame[i][1], stationary[i] * z_scale};
		positions.push_back(json_object({{"record_id", visual::quote(node_id(i))},
										 {"position", visual::number_array(position)}}));
	}
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(dataset_id)},
						{"space_id", visual::quote(space_id)},
						{"name", visual::quote(name)},
						{"dimension", "3"},
						{"record_positions", visual::array_of(positions)},
						{"metadata",
						 json_object({{"source_operator", visual::quote(source_operator)},
									  {"step", json_size(step)},
									  {"z_channel", visual::quote("stationary distribution scaled for display")},
									  {"z_scale", visual::num(z_scale)}})}});
}

auto relation_json(const std::string &id, const std::string &dataset_id, const std::string &name,
				   const std::string &relation_type, const std::vector<std::string> &record_ids,
				   const Matrix &values, const std::string &metadata_json) -> std::string
{
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(dataset_id)},
						{"name", visual::quote(name)},
						{"relation_type", visual::quote(relation_type)},
						{"value_type", visual::quote("scalar")},
						{"record_ids", visual::string_array(record_ids)},
						{"storage", visual::quote("dense_matrix")},
						{"values", dense_matrix_json(values)},
						{"metadata", metadata_json}});
}

auto max_abs_diagonal(const Matrix &matrix) -> double
{
	double value = 0.0;
	for (std::size_t i = 0; i < matrix.size(); ++i) {
		value = std::max(value, std::abs(matrix[i][i]));
	}
	return value;
}

auto max_symmetry_error(const Matrix &matrix) -> double
{
	double value = 0.0;
	for (std::size_t i = 0; i < matrix.size(); ++i) {
		for (std::size_t j = i + 1; j < matrix.size(); ++j) {
			value = std::max(value, std::abs(matrix[i][j] - matrix[j][i]));
		}
	}
	return value;
}

auto max_triangle_violation(const Matrix &matrix) -> double
{
	double value = 0.0;
	const std::size_t n = matrix.size();
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			for (std::size_t k = 0; k < n; ++k) {
				value = std::max(value, matrix[i][k] - matrix[i][j] - matrix[j][k]);
			}
		}
	}
	return value;
}

auto max_row_sum_error(const mtrc::MetricTransition &transition) -> double
{
	double value = 0.0;
	for (std::size_t i = 0; i < transition.node_count; ++i) {
		value = std::max(value, std::abs(transition.row_sum(i) - 1.0));
	}
	return value;
}

auto min_transition_probability(const mtrc::MetricTransition &transition) -> double
{
	double value = std::numeric_limits<double>::infinity();
	for (const auto &row : transition.probabilities) {
		for (const auto probability : row) {
			value = std::min(value, probability);
		}
	}
	return std::isfinite(value) ? value : 0.0;
}

auto constant_fixed_point_error(const mtrc::MetricTransition &transition, double constant) -> double
{
	double value = 0.0;
	for (std::size_t i = 0; i < transition.node_count; ++i) {
		double mixed = 0.0;
		for (std::size_t j = 0; j < transition.node_count; ++j) {
			mixed += transition.probabilities[i][j] * constant;
		}
		value = std::max(value, std::abs(mixed - constant));
	}
	return value;
}

auto stationary_sum_error(const mtrc::MetricTransition &transition) -> double
{
	double total = 0.0;
	for (const auto value : transition.stationary) {
		total += value;
	}
	return std::abs(total - 1.0);
}

auto stationary_invariance_l1(const mtrc::MetricTransition &transition) -> double
{
	std::vector<double> pushed(transition.node_count, 0.0);
	for (std::size_t j = 0; j < transition.node_count; ++j) {
		for (std::size_t i = 0; i < transition.node_count; ++i) {
			pushed[j] += transition.stationary[i] * transition.probabilities[i][j];
		}
	}
	double drift = 0.0;
	for (std::size_t i = 0; i < transition.node_count; ++i) {
		drift += std::abs(pushed[i] - transition.stationary[i]);
	}
	return drift;
}

auto transition_max_difference(const mtrc::MetricTransition &lhs, const mtrc::MetricTransition &rhs) -> double
{
	double value = 0.0;
	for (std::size_t i = 0; i < lhs.node_count; ++i) {
		for (std::size_t j = 0; j < lhs.node_count; ++j) {
			value = std::max(value, std::abs(lhs.probabilities[i][j] - rhs.probabilities[i][j]));
		}
	}
	return value;
}

auto reverse_max_principle_holds(const std::vector<Signal> &frames, std::size_t dimension) -> bool
{
	if (frames.empty() || frames.front().empty()) {
		return true;
	}
	for (std::size_t c = 0; c < dimension; ++c) {
		double lo = frames.front()[0][c];
		double hi = lo;
		for (const auto &node : frames.front()) {
			lo = std::min(lo, node[c]);
			hi = std::max(hi, node[c]);
		}
		for (const auto &frame : frames) {
			for (const auto &node : frame) {
				if (node[c] < lo - 1e-9 || node[c] > hi + 1e-9) {
					return false;
				}
			}
		}
	}
	return true;
}

auto max_energy_increase(const std::vector<double> &energy) -> double
{
	double value = 0.0;
	for (std::size_t i = 1; i < energy.size(); ++i) {
		value = std::max(value, energy[i] - energy[i - 1]);
	}
	return value;
}

auto native_check(std::vector<std::string> &diagnostics, const std::string &id, const std::string &message,
				  bool pass, double value, const std::string &comparison, double threshold) -> bool
{
	diagnostics.push_back(json_object({{"id", visual::quote(id)},
									   {"kind", visual::quote("native-invariant")},
									   {"status", visual::quote(pass ? "pass" : "fail")},
									   {"native_example_check", visual::quote(message)},
									   {"value", visual::num(value)},
									   {"comparison", visual::quote(comparison)},
									   {"threshold", visual::num(threshold)},
									   {"source", visual::quote("examples/engine/finite_metric_dynamics.cpp")}}));
	return pass;
}

auto native_boolean_check(std::vector<std::string> &diagnostics, const std::string &id, const std::string &message,
						  bool pass) -> bool
{
	diagnostics.push_back(json_object({{"id", visual::quote(id)},
									   {"kind", visual::quote("native-invariant")},
									   {"status", visual::quote(pass ? "pass" : "fail")},
									   {"native_example_check", visual::quote(message)},
									   {"value", json_bool(pass)},
									   {"expected", "true"},
									   {"source", visual::quote("examples/engine/finite_metric_dynamics.cpp")}}));
	return pass;
}

auto build_visual_document() -> std::string
{
	const std::string dataset_id = "finite-metric-dynamics";
	const std::string space_id = "finite-dynamics-space";
	const std::string metric_relation_id = "finite-euclidean-metric";
	const std::string transition_relation_id = "geometry-derived-transition";
	const std::string forward_timeline_id = "forward-diffusion";
	const std::string reverse_timeline_id = "reverse-reconstruction";

	const auto records = finite_metric_dynamics_fixture::make_records();
	const Signal clean = signal_of_records(records);
	auto space = mtrc::make_space(records, mtrc::Euclidean<double>{});
	const std::size_t n = space.size();

	mtrc::DynamicsSchedule structure;
	structure.neighbors = 4;
	structure.bandwidth = 0.0;

	const auto distances = native_distance_matrix(space);
	const auto transition = mtrc::metric_transition(space, structure);

	const std::uint64_t walk_seed = 20240617;
	const auto walk = mtrc::metric_random_walk(transition, 0, 4000, walk_seed, 16);
	const auto walk_again = mtrc::metric_random_walk(transition, 0, 4000, walk_seed, 16);
	const auto walk_other = mtrc::metric_random_walk(transition, 0, 4000, walk_seed + 1, 16);

	mtrc::DynamicsSchedule forward = structure;
	forward.steps = 6;
	forward.diffusivity = 0.05;
	forward.noise_scale = 0.06;
	forward.seed = 1234;
	const auto forward_run = mtrc::metric_diffuse(space, forward, transition);
	const auto forward_repeat = mtrc::metric_diffuse(space, forward, transition);
	mtrc::DynamicsSchedule forward_other = forward;
	forward_other.seed = 9999;
	const auto forward_other_run = mtrc::metric_diffuse(space, forward_other, transition);

	const Signal degraded = signal_of_space(forward_run.result.space);
	const double degraded_mse = mse(degraded, clean);

	mtrc::DynamicsSchedule reverse = structure;
	reverse.steps = 40;
	reverse.diffusivity = 0.18;
	reverse.noise_scale = 0.0;
	const auto reverse_run = mtrc::metric_reconstruct(forward_run.result.space, reverse, transition);

	double best_mse = degraded_mse;
	std::size_t best_step = 0;
	for (std::size_t t = 0; t < reverse_run.frames.size(); ++t) {
		const double value = mse(reverse_run.frames[t], clean);
		if (value < best_mse) {
			best_mse = value;
			best_step = t;
		}
	}

	const auto adaptive_run = mtrc::metric_reconstruct(forward_run.result.space, reverse);
	double adaptive_best = degraded_mse;
	std::size_t adaptive_step = 0;
	for (std::size_t t = 0; t < adaptive_run.frames.size(); ++t) {
		const double value = mse(adaptive_run.frames[t], clean);
		if (value < adaptive_best) {
			adaptive_best = value;
			adaptive_step = t;
		}
	}

	auto shifted_records = records;
	for (auto &record : shifted_records) {
		record[0] += 10.0;
		record[1] -= 4.0;
	}
	auto shifted_space = mtrc::make_space(shifted_records, mtrc::Euclidean<double>{});
	const auto shifted_transition = mtrc::metric_transition(shifted_space, structure);

	std::vector<std::string> record_ids;
	record_ids.reserve(n);
	for (std::size_t i = 0; i < n; ++i) {
		record_ids.push_back(node_id(i));
	}

	std::vector<std::string> diagnostics;
	bool native_checks_pass = true;
	const double metric_diag_error = max_abs_diagonal(distances);
	const double metric_symmetry_error = max_symmetry_error(distances);
	const double metric_triangle_violation = max_triangle_violation(distances);
	const double row_error = max_row_sum_error(transition);
	const double min_probability = min_transition_probability(transition);
	const double fixed_point_error = constant_fixed_point_error(transition, 3.5);
	const double stationary_sum = stationary_sum_error(transition);
	const double stationary_l1 = stationary_invariance_l1(transition);
	const double shifted_max_diff = transition_max_difference(transition, shifted_transition);
	const double reverse_max_energy_delta = max_energy_increase(reverse_run.dirichlet_energy);

	native_checks_pass &= native_boolean_check(diagnostics, "check-fixture-node-count", "fixture node count",
											   n == finite_metric_dynamics_fixture::node_count);
	native_checks_pass &= native_check(diagnostics, "check-metric-diagonal", "metric diagonal is zero",
									   metric_diag_error < 1e-12, metric_diag_error, "<", 1e-12);
	native_checks_pass &= native_check(diagnostics, "check-metric-symmetry", "metric is symmetric",
									   metric_symmetry_error < 1e-12, metric_symmetry_error, "<", 1e-12);
	native_checks_pass &= native_check(diagnostics, "check-metric-triangle", "metric triangle inequality",
									   metric_triangle_violation < 1e-12, metric_triangle_violation, "<", 1e-12);
	native_checks_pass &= native_check(diagnostics, "check-transition-row-stochastic",
									   "row of P sums to 1 (probability from geometry)", row_error < 1e-9,
									   row_error, "<", 1e-9);
	native_checks_pass &= native_check(diagnostics, "check-transition-nonnegative",
									   "transition probabilities are non-negative", min_probability >= 0.0,
									   min_probability, ">=", 0.0);
	native_checks_pass &= native_check(diagnostics, "check-constant-fixed-point",
									   "constant signal is a fixed point of diffusion", fixed_point_error < 1e-9,
									   fixed_point_error, "<", 1e-9);
	native_checks_pass &= native_check(diagnostics, "check-stationary-sum",
									   "stationary distribution sums to 1", stationary_sum < 1e-9,
									   stationary_sum, "<", 1e-9);
	native_checks_pass &= native_check(diagnostics, "check-stationary-invariant",
									   "stationary distribution is invariant under P (pi P = pi)",
									   stationary_l1 < 1e-9, stationary_l1, "<", 1e-9);
	native_checks_pass &= native_check(diagnostics, "check-transition-translation-invariant",
									   "P is invariant under distance-preserving record changes",
									   shifted_max_diff < 1e-12, shifted_max_diff, "<", 1e-12);
	native_checks_pass &= native_check(diagnostics, "check-walk-converges",
									   "random walk converges to the stationary distribution derived from geometry",
									   walk.total_variation_to_stationary < 0.06,
									   walk.total_variation_to_stationary, "<", 0.06);
	native_checks_pass &= native_boolean_check(diagnostics, "check-walk-reproducible",
											   "random walk is reproducible for a fixed seed",
											   walk.visit_counts == walk_again.visit_counts);
	native_checks_pass &= native_boolean_check(diagnostics, "check-walk-different-seed",
											   "a different seed yields a different sample path",
											   walk.visit_counts != walk_other.visit_counts);
	native_checks_pass &= native_boolean_check(diagnostics, "check-forward-shape",
											   "forward trajectory has steps+1 frames",
											   forward_run.frames.size() == forward.steps + 1);
	native_checks_pass &= native_boolean_check(diagnostics, "check-forward-lineage",
											   "forward lineage has one row per derived record",
											   forward_run.result.source_record_count == n &&
												   forward_run.result.source_records.size() == n &&
												   forward_run.result.representative_records.size() == n &&
												   forward_run.result.mapping == "metric_diffusion_forward");
	native_checks_pass &= native_boolean_check(diagnostics, "check-forward-reproducible",
											   "forward diffusion is reproducible for a fixed seed",
											   signals_equal(signal_of_space(forward_run.result.space),
															 signal_of_space(forward_repeat.result.space)));
	native_checks_pass &= native_boolean_check(diagnostics, "check-forward-different-seed",
											   "a different seed degrades the space differently",
											   !signals_equal(signal_of_space(forward_run.result.space),
															  signal_of_space(forward_other_run.result.space)));
	native_checks_pass &= native_check(diagnostics, "check-forward-degraded",
									   "forward diffusion degraded the signal", degraded_mse > 0.0, degraded_mse,
									   ">", 0.0);
	native_checks_pass &= native_boolean_check(diagnostics, "check-forward-energy-increase",
											   "noise raised the graph Dirichlet energy",
											   forward_run.dirichlet_energy.back() > forward_run.dirichlet_energy.front());
	native_checks_pass &= native_boolean_check(diagnostics, "check-reverse-shape",
											   "reverse trajectory has steps+1 frames",
											   reverse_run.frames.size() == reverse.steps + 1 &&
												   reverse_run.result.size() == n &&
												   reverse_run.result.mapping == "metric_diffusion_reverse");
	native_checks_pass &= native_boolean_check(diagnostics, "check-reverse-maximum-principle",
											   "reverse diffusion obeys the maximum principle",
											   reverse_max_principle_holds(reverse_run.frames,
																		   finite_metric_dynamics_fixture::dimension));
	native_checks_pass &= native_check(diagnostics, "check-reverse-energy-monotone",
									   "reverse diffusion does not increase the Dirichlet energy",
									   reverse_max_energy_delta <= 1e-9, reverse_max_energy_delta, "<=", 1e-9);
	native_checks_pass &= native_boolean_check(diagnostics, "check-reconstruction-improves",
											   "reconstruction improves on the degraded input", best_step > 0);
	native_checks_pass &= native_check(diagnostics, "check-reconstruction-mse-reduction",
									   "reverse diffusion recovers structure (>= 15% MSE reduction)",
									   best_mse < 0.85 * degraded_mse, best_mse / degraded_mse, "<", 0.85);
	native_checks_pass &= native_boolean_check(diagnostics, "check-adaptive-reconstruction-improves",
											   "reconstruction from degraded geometry alone still improves",
											   adaptive_step > 0 && adaptive_best < degraded_mse);

	std::vector<std::string> datasets;
	datasets.push_back(json_object(
		{{"id", visual::quote(dataset_id)},
		 {"title", visual::quote("Finite Metric Dynamics native export")},
		 {"description",
		  visual::quote("Native C++ evidence for geometry-derived probability, forward diffusion and reverse reconstruction over a deterministic finite metric fixture.")},
		 {"source", visual::quote("METRIC C++ examples/engine/finite_metric_dynamics_fixture.hpp")},
		 {"license", visual::quote("MPL-2.0")},
		 {"metadata",
		  json_object({{"native_example", visual::quote("examples/engine/finite_metric_dynamics.cpp")},
					   {"node_count", json_size(n)},
					   {"dimension", json_size(finite_metric_dynamics_fixture::dimension)},
					   {"public_hero_ready", "false"}})}}));

	std::vector<std::string> records_json;
	records_json.reserve(n);
	for (std::size_t i = 0; i < n; ++i) {
		const auto payload = json_object({{"kind", visual::quote("vector")},
										  {"values", visual::number_array(records[i])},
										  {"fixture", visual::quote("s_curve_y_equals_sin_x")},
										  {"index", json_size(i)},
										  {"clean_state", "true"}});
		records_json.push_back(json_object({{"id", visual::quote(record_ids[i])},
											{"dataset_id", visual::quote(dataset_id)},
											{"record_type", visual::quote("vector")},
											{"label", visual::quote("fixture node " + std::to_string(i))},
											{"payload", payload}}));
	}

	const auto metric_metadata =
		json_object({{"computed_by", visual::quote("mtrc::MetricSpace::distance with mtrc::Euclidean<double>")},
					 {"law_check",
					  json_object({{"status", visual::quote(metric_triangle_violation < 1e-12 ? "checked_pass" : "checked_fail")},
								   {"diagonal_max_abs", visual::num(metric_diag_error)},
								   {"symmetry_max_abs", visual::num(metric_symmetry_error)},
								   {"triangle_max_violation", visual::num(metric_triangle_violation)}})}});

	const auto transition_metadata =
		json_object({{"computed_by", visual::quote("mtrc::metric_transition")},
					 {"neighbors", json_size(transition.neighbors)},
					 {"bandwidth", visual::num(transition.bandwidth)},
					 {"row_stochastic_max_abs_error", visual::num(row_error)},
					 {"stationary_entropy_nats", visual::num(entropy(transition.stationary))},
					 {"stationary_invariance_l1", visual::num(stationary_l1)}});

	std::vector<std::string> relations;
	relations.push_back(relation_json(metric_relation_id, dataset_id, "Euclidean fixture metric", "metric",
									  record_ids, distances, metric_metadata));
	relations.push_back(relation_json(transition_relation_id, dataset_id, "Geometry-derived transition P",
									  "transition", record_ids, transition.probabilities, transition_metadata));

	std::vector<std::string> spaces;
	spaces.push_back(json_object(
		{{"id", visual::quote(space_id)},
		 {"dataset_id", visual::quote(dataset_id)},
		 {"record_ids", visual::string_array(record_ids)},
		 {"primary_relation_id", visual::quote(metric_relation_id)},
		 {"space_type", visual::quote("finite_metric_space")},
		 {"metadata",
		  json_object({{"native_fixture", visual::quote("finite_metric_dynamics_fixture.hpp")},
					   {"metric_relation_id", visual::quote(metric_relation_id)},
					   {"transition_relation_id", visual::quote(transition_relation_id)},
					   {"foundation_export", "true"},
					   {"public_hero_ready", "false"}})}}));

	std::vector<double> forward_terminal_error;
	std::vector<double> best_reconstruction_error;
	std::vector<double> terminal_reconstruction_error;
	std::vector<double> reconstruction_improvement;
	forward_terminal_error.reserve(n);
	best_reconstruction_error.reserve(n);
	terminal_reconstruction_error.reserve(n);
	reconstruction_improvement.reserve(n);
	for (std::size_t i = 0; i < n; ++i) {
		const double fwd = node_mse(forward_run.frames.back()[i], clean[i]);
		const double best = node_mse(reverse_run.frames[best_step][i], clean[i]);
		const double terminal = node_mse(reverse_run.frames.back()[i], clean[i]);
		forward_terminal_error.push_back(fwd);
		best_reconstruction_error.push_back(best);
		terminal_reconstruction_error.push_back(terminal);
		reconstruction_improvement.push_back(fwd - best);
	}

	std::vector<std::string> properties;
	properties.push_back(scalar_record_property("stationary-distribution", dataset_id, "stationary distribution",
												record_ids, transition.stationary,
												"Native stationary distribution pi derived from metric degrees."));
	properties.push_back(scalar_record_property("transition-degree", dataset_id, "transition graph degree",
												record_ids, transition.degree,
												"Symmetric heat-kernel degree before row normalization."));
	properties.push_back(scalar_record_property("walk-empirical-distribution", dataset_id,
												"random walk empirical distribution", record_ids, walk.empirical,
												"Visit distribution from native metric_random_walk."));
	properties.push_back(scalar_record_property_from_size_t("walk-visit-count", dataset_id, "random walk visit count",
															record_ids, walk.visit_counts,
															"Raw visit counts from the seeded native random walk."));
	properties.push_back(scalar_record_property("forward-terminal-error", dataset_id,
												"forward terminal MSE to clean", record_ids,
												forward_terminal_error,
												"Per-record squared error after native forward diffusion."));
	properties.push_back(scalar_record_property("best-reconstruction-error", dataset_id,
												"best reverse MSE to clean", record_ids,
												best_reconstruction_error,
												"Per-record squared error at the best native reverse step."));
	properties.push_back(scalar_record_property("terminal-reconstruction-error", dataset_id,
												"terminal reverse MSE to clean", record_ids,
												terminal_reconstruction_error,
												"Per-record squared error at the terminal reverse step."));
	properties.push_back(scalar_record_property("reconstruction-improvement", dataset_id,
												"best reconstruction improvement", record_ids,
												reconstruction_improvement,
												"Per-record forward terminal error minus best reverse error."));
	properties.push_back(timeline_step_property("forward-dirichlet-energy", dataset_id,
												"forward Dirichlet energy", forward_timeline_id,
												forward_run.dirichlet_energy,
												"Graph Dirichlet energy from native forward diffusion frames."));
	properties.push_back(timeline_step_property("reverse-dirichlet-energy", dataset_id,
												"reverse Dirichlet energy", reverse_timeline_id,
												reverse_run.dirichlet_energy,
												"Graph Dirichlet energy from native reverse reconstruction frames."));
	properties.push_back(timeline_step_property_from_signal("forward-mse-to-clean", dataset_id,
															"forward MSE to clean", forward_timeline_id,
															forward_run.frames, clean,
															"Per-step MSE of native forward trajectory to the clean fixture."));
	properties.push_back(timeline_step_property_from_signal("reverse-mse-to-clean", dataset_id,
															"reverse MSE to clean", reverse_timeline_id,
															reverse_run.frames, clean,
															"Per-step MSE of native reverse trajectory to the clean fixture."));

	std::vector<std::string> graph_edges;
	std::size_t edge_index = 0;
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			const double probability = transition.probabilities[i][j];
			if (probability <= 0.0) {
				continue;
			}
			graph_edges.push_back(json_object({{"id", visual::quote("transition-edge-" + std::to_string(edge_index++))},
											   {"source", visual::quote(record_ids[i])},
											   {"target", visual::quote(record_ids[j])},
											   {"value", visual::num(probability)},
											   {"probability", visual::num(probability)},
											   {"affinity", visual::num(transition.affinity[i][j])},
											   {"distance", visual::num(distances[i][j])}}));
		}
	}

	std::vector<std::string> graphs;
	graphs.push_back(json_object({{"id", visual::quote("transition-graph")},
								  {"dataset_id", visual::quote(dataset_id)},
								  {"node_record_ids", visual::string_array(record_ids)},
								  {"edge_relation_id", visual::quote(transition_relation_id)},
								  {"graph_type", visual::quote("transition")},
								  {"edges", visual::array_of(graph_edges)},
								  {"metadata",
								   json_object({{"computed_by", visual::quote("mtrc::metric_transition")},
												{"edge_count", json_size(graph_edges.size())},
												{"directed", "true"},
												{"neighbors", json_size(transition.neighbors)}})}}));

	std::vector<std::string> coordinates;
	const double z_scale = 8.0;
	for (std::size_t t = 0; t < forward_run.frames.size(); ++t) {
		coordinates.push_back(coordinate_json(coordinate_id("forward", t), dataset_id, space_id,
											  "forward diffusion step " + std::to_string(t), forward_run.frames[t],
											  transition.stationary, z_scale, "metric_diffusion_forward", t));
	}
	for (std::size_t t = 0; t < reverse_run.frames.size(); ++t) {
		coordinates.push_back(coordinate_json(coordinate_id("reverse", t), dataset_id, space_id,
											  "reverse reconstruction step " + std::to_string(t),
											  reverse_run.frames[t], transition.stationary, z_scale,
											  "metric_diffusion_reverse", t));
	}

	std::vector<std::string> forward_steps;
	forward_steps.reserve(forward_run.frames.size());
	for (std::size_t t = 0; t < forward_run.frames.size(); ++t) {
		forward_steps.push_back(json_object({{"id", visual::quote(step_id(forward_timeline_id, t))},
											 {"index", json_size(t)},
											 {"t", visual::num(static_cast<double>(t) / static_cast<double>(forward.steps))},
											 {"name", visual::quote("forward diffusion step " + std::to_string(t))},
											 {"coordinate_id", visual::quote(coordinate_id("forward", t))},
											 {"relation_id", visual::quote(transition_relation_id)},
											 {"dirichlet_energy", visual::num(forward_run.dirichlet_energy[t])},
											 {"mse_to_clean", visual::num(mse(forward_run.frames[t], clean))}}));
	}

	std::vector<std::string> reverse_steps;
	reverse_steps.reserve(reverse_run.frames.size());
	for (std::size_t t = 0; t < reverse_run.frames.size(); ++t) {
		reverse_steps.push_back(json_object({{"id", visual::quote(step_id(reverse_timeline_id, t))},
											 {"index", json_size(t)},
											 {"t", visual::num(static_cast<double>(t) / static_cast<double>(reverse.steps))},
											 {"name", visual::quote("reverse reconstruction step " + std::to_string(t))},
											 {"coordinate_id", visual::quote(coordinate_id("reverse", t))},
											 {"relation_id", visual::quote(transition_relation_id)},
											 {"dirichlet_energy", visual::num(reverse_run.dirichlet_energy[t])},
											 {"mse_to_clean", visual::num(mse(reverse_run.frames[t], clean))},
											 {"is_best_step", json_bool(t == best_step)}}));
	}

	std::vector<std::string> timelines;
	timelines.push_back(json_object({{"id", visual::quote(forward_timeline_id)},
									 {"dataset_id", visual::quote(dataset_id)},
									 {"name", visual::quote("forward diffusion trajectory")},
									 {"space_id", visual::quote(space_id)},
									 {"timeline_type", visual::quote("diffusion_forward")},
									 {"steps", visual::array_of(forward_steps)},
									 {"metadata",
									  json_object({{"operator", visual::quote(forward_run.operator_name)},
												   {"direction", visual::quote(forward_run.direction)},
												   {"steps", json_size(forward.steps)},
												   {"diffusivity", visual::num(forward.diffusivity)},
												   {"noise_scale", visual::num(forward.noise_scale)},
												   {"seed", std::to_string(forward.seed)}})}}));
	timelines.push_back(json_object({{"id", visual::quote(reverse_timeline_id)},
									 {"dataset_id", visual::quote(dataset_id)},
									 {"name", visual::quote("reverse reconstruction trajectory")},
									 {"space_id", visual::quote(space_id)},
									 {"timeline_type", visual::quote("diffusion_reverse")},
									 {"steps", visual::array_of(reverse_steps)},
									 {"metadata",
									  json_object({{"operator", visual::quote(reverse_run.operator_name)},
												   {"direction", visual::quote(reverse_run.direction)},
												   {"steps", json_size(reverse.steps)},
												   {"diffusivity", visual::num(reverse.diffusivity)},
												   {"noise_scale", visual::num(reverse.noise_scale)},
												   {"best_step", json_size(best_step)},
												   {"best_mse", visual::num(best_mse)},
												   {"degraded_mse", visual::num(degraded_mse)},
												   {"improvement_fraction", visual::num(1.0 - best_mse / degraded_mse)}})}}));

	std::vector<std::string> events;
	events.push_back(json_object({{"id", visual::quote("event-random-walk-convergence")},
								  {"dataset_id", visual::quote(dataset_id)},
								  {"event_type", visual::quote("random_walk")},
								  {"payload",
								   json_object({{"seed", std::to_string(walk.seed)},
												{"start", json_size(walk.start)},
												{"steps", json_size(walk.steps)},
												{"walkers", json_size(walk.walkers)},
												{"total_variation_to_stationary",
												 visual::num(walk.total_variation_to_stationary)},
												{"visit_counts", integer_array_json(walk.visit_counts)}})}}));
	events.push_back(json_object({{"id", visual::quote("event-best-reconstruction")},
								  {"dataset_id", visual::quote(dataset_id)},
								  {"event_type", visual::quote("reconstruction_summary")},
								  {"payload",
								   json_object({{"best_step", json_size(best_step)},
												{"best_mse", visual::num(best_mse)},
												{"degraded_mse", visual::num(degraded_mse)},
												{"adaptive_best_step", json_size(adaptive_step)},
												{"adaptive_best_mse", visual::num(adaptive_best)},
												{"improvement_fraction", visual::num(1.0 - best_mse / degraded_mse)},
												{"adaptive_improvement_fraction",
												 visual::num(1.0 - adaptive_best / degraded_mse)}})}}));

	std::vector<std::string> views;
	views.push_back(json_object({{"id", visual::quote("view-clean-space")},
								 {"kind", visual::quote("metric-space")},
								 {"name", visual::quote("Clean finite metric space")},
								 {"spaceId", visual::quote(space_id)},
								 {"coordinateId", visual::quote("coord-forward-00")},
								 {"propertyId", visual::quote("stationary-distribution")}}));
	views.push_back(json_object({{"id", visual::quote("view-forward-dynamics")},
								 {"kind", visual::quote("dynamics")},
								 {"name", visual::quote("Forward diffusion trajectory")},
								 {"spaceId", visual::quote(space_id)},
								 {"timelineId", visual::quote(forward_timeline_id)},
								 {"coordinateId", visual::quote("coord-forward-06")},
								 {"propertyId", visual::quote("forward-terminal-error")}}));
	views.push_back(json_object({{"id", visual::quote("view-reverse-dynamics")},
								 {"kind", visual::quote("dynamics")},
								 {"name", visual::quote("Reverse reconstruction trajectory")},
								 {"spaceId", visual::quote(space_id)},
								 {"timelineId", visual::quote(reverse_timeline_id)},
								 {"coordinateId", visual::quote(coordinate_id("reverse", best_step))},
								 {"propertyId", visual::quote("best-reconstruction-error")}}));
	views.push_back(json_object({{"id", visual::quote("view-transition-matrix")},
								 {"kind", visual::quote("relation-matrix")},
								 {"name", visual::quote("Geometry-derived transition matrix")},
								 {"relationId", visual::quote(transition_relation_id)}}));

	const auto provenance = json_object(
		{{"writer", visual::quote("examples/engine/finite_metric_dynamics_visual_export.cpp")},
		 {"generator", visual::quote("native-cpp")},
		 {"computed_by", visual::quote("METRIC C++ mtrc::metric_transition/metric_diffuse/metric_reconstruct")},
		 {"fixture", visual::quote("examples/engine/finite_metric_dynamics_fixture.hpp")},
		 {"synthetic_js", "false"},
		 {"native_export", "true"},
		 {"public_hero_ready", "false"},
		 {"native_checks_pass", json_bool(native_checks_pass)},
		 {"native_check_count", json_size(diagnostics.size())},
		 {"schema_note", visual::quote("Native export foundation; no public hero readiness claimed.")}});

	return json_object({{"schema", visual::quote("metric.visual.v1")},
						{"provenance", provenance},
						{"datasets", visual::array_of(datasets)},
						{"records", visual::array_of(records_json)},
						{"relations", visual::array_of(relations)},
						{"spaces", visual::array_of(spaces)},
						{"properties", visual::array_of(properties)},
						{"graphs", visual::array_of(graphs)},
						{"coordinates", visual::array_of(coordinates)},
						{"timelines", visual::array_of(timelines)},
						{"events", visual::array_of(events)},
						{"views", visual::array_of(views)},
						{"diagnostics", visual::array_of(diagnostics)}});
}

auto usage(const char *program) -> std::string
{
	std::ostringstream out;
	out << "Usage: " << program << " [--export-dir <dir>]\n"
		<< "Default: write metric.visual.v1 JSON to stdout.\n"
		<< "--export-dir writes metric.visual.json under the given directory.\n";
	return out.str();
}

} // namespace

int main(int argc, char **argv)
{
	std::string export_dir;
	for (int i = 1; i < argc; ++i) {
		const std::string arg = argv[i];
		if (arg == "--help" || arg == "-h") {
			std::cout << usage(argv[0]);
			return 0;
		}
		if (arg == "--export-dir") {
			if (i + 1 >= argc) {
				std::cerr << "--export-dir requires a directory\n";
				return 2;
			}
			export_dir = argv[++i];
			continue;
		}
		std::cerr << "Unknown argument: " << arg << "\n" << usage(argv[0]);
		return 2;
	}

	const std::string document = build_visual_document();
	if (export_dir.empty()) {
		std::cout << document << "\n";
		return 0;
	}

	const std::filesystem::path output_dir(export_dir);
	std::error_code ec;
	std::filesystem::create_directories(output_dir, ec);
	if (ec) {
		std::cerr << "Failed to create export directory " << output_dir << ": " << ec.message() << "\n";
		return 1;
	}

	const auto output_path = output_dir / "metric.visual.json";
	std::ofstream out(output_path);
	if (!out) {
		std::cerr << "Failed to open " << output_path << " for writing\n";
		return 1;
	}
	out << document << "\n";
	if (!out) {
		std::cerr << "Failed to write " << output_path << "\n";
		return 1;
	}

	std::cerr << "Wrote " << output_path << "\n";
	return 0;
}
