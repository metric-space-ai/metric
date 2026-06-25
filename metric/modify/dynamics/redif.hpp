// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MODIFY_DYNAMICS_REDIF_HPP
#define _METRIC_MODIFY_DYNAMICS_REDIF_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/result.hpp>
#include <metric/metric/catalog/structured/EMD.hpp>
#include <metric/modify/dynamics/finite_dynamics.hpp>
#include <metric/record/id.hpp>

namespace mtrc::modify::dynamics {

struct redif_options {
	std::size_t neighbors = 10;
	std::size_t iterations = 15;
	double euler_step = 0.25;
	bool adaptive_geometry = true;
	std::size_t max_dense_records = default_metric_transition_max_dense_records;
};

struct EntropyTrajectory {
	std::vector<double> shannon_entropy;
	std::vector<double> relative_entropy_to_stationary;

	auto size() const -> std::size_t { return shannon_entropy.size(); }
	auto empty() const -> bool { return shannon_entropy.empty(); }
};

struct RedifOperatorDiagnostics {
	std::size_t node_count{};
	std::size_t neighbors{};
	double minimum_degree{};
	double maximum_degree{};
	double minimum_transition_row_sum{};
	double maximum_transition_row_sum{};
	bool reversible{true};
};

struct RedifOperator {
	std::size_t node_count{};
	std::size_t neighbors{};
	std::vector<std::vector<double>> local_distances;
	std::vector<std::vector<double>> affinity;
	std::vector<double> degree;
	std::vector<std::vector<double>> laplacian;
	std::vector<std::vector<double>> transition;
	std::vector<double> stationary;
	RedifOperatorDiagnostics diagnostics;

	auto size() const -> std::size_t { return node_count; }

	auto transition_row_sum(std::size_t row) const -> double
	{
		double total = 0.0;
		for (const auto value : transition[row]) {
			total += value;
		}
		return total;
	}
};

struct RedifEntropyDiagnostics {
	RecordId id;
	double initial_shannon_entropy{};
	double terminal_shannon_entropy{};
	double initial_relative_entropy_to_stationary{};
	double terminal_relative_entropy_to_stationary{};
};

struct RedifMeasurePath {
	RecordId id;
	std::vector<std::vector<double>> measures;
	std::vector<double> step_transport;
	double transport_path_length = 0.0;

	auto terminal_measure() const -> const std::vector<double> &
	{
		if (measures.empty()) {
			throw std::logic_error("RedifMeasurePath has no terminal measure");
		}
		return measures.back();
	}
};

struct RedifMeasureResult {
	std::vector<RedifMeasurePath> paths;
	std::size_t record_count{};
	std::size_t neighbors{};
	std::size_t iterations{};
	double euler_step{};
	bool adaptive_geometry{};
	bool exact{true};
	std::string operator_name;
	std::string strategy;
	std::string representation;
	std::vector<double> initial_stationary;
	std::vector<RedifEntropyDiagnostics> entropy_diagnostics;
	std::vector<RedifOperatorDiagnostics> operator_diagnostics;

	auto size() const -> std::size_t { return paths.size(); }
	auto empty() const -> bool { return paths.empty(); }
	auto begin() const -> std::vector<RedifMeasurePath>::const_iterator { return paths.begin(); }
	auto end() const -> std::vector<RedifMeasurePath>::const_iterator { return paths.end(); }
	auto operator[](std::size_t index) const -> const RedifMeasurePath & { return paths[index]; }
};

namespace detail {

inline auto validate_redif_options(std::size_t record_count, const redif_options &options) -> void
{
	if (record_count == 0) {
		throw std::invalid_argument("Redif requires a non-empty metric space");
	}
	if (options.iterations == 0) {
		throw std::invalid_argument("Redif iterations must be positive");
	}
	if (!(options.euler_step > 0.0) || !std::isfinite(options.euler_step)) {
		throw std::invalid_argument("Redif euler_step must be finite and positive");
	}
	if (options.euler_step > 1.0) {
		throw std::invalid_argument("Redif euler_step must be <= 1 for probability-measure dynamics");
	}
	if (record_count > 1 && options.neighbors == 0) {
		throw std::invalid_argument("Redif neighbors must be positive for spaces with more than one record");
	}
	if (options.max_dense_records > 0 && record_count > options.max_dense_records) {
		throw MetricInputError("Redif dense construction exceeds max_dense_records: records=" +
							   std::to_string(record_count) + " max_dense_records=" +
							   std::to_string(options.max_dense_records));
	}
}

inline auto identity_measures(std::size_t count) -> std::vector<std::vector<double>>
{
	std::vector<std::vector<double>> state(count, std::vector<double>(count, 0.0));
	for (std::size_t i = 0; i < count; ++i) {
		state[i][i] = 1.0;
	}
	return state;
}

inline auto local_knn_distance_matrix(const std::vector<std::vector<double>> &distances, std::size_t neighbors)
	-> std::vector<std::vector<double>>
{
	const std::size_t n = distances.size();
	const std::size_t k = std::min(neighbors, n > 0 ? n - 1 : 0);
	std::vector<std::vector<double>> local(n, std::vector<double>(n, 0.0));

	for (std::size_t i = 0; i < n; ++i) {
		std::vector<std::size_t> order;
		order.reserve(n);
		for (std::size_t j = 0; j < n; ++j) {
			if (j != i) {
				order.push_back(j);
			}
		}
		std::sort(order.begin(), order.end(), [&](std::size_t lhs, std::size_t rhs) {
			if (distances[i][lhs] != distances[i][rhs]) {
				return distances[i][lhs] < distances[i][rhs];
			}
			return lhs < rhs;
		});
		for (std::size_t t = 0; t < k; ++t) {
			local[i][order[t]] = distances[i][order[t]];
		}
	}

	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = i + 1; j < n; ++j) {
			const auto value = std::max(local[i][j], local[j][i]);
			local[i][j] = value;
			local[j][i] = value;
		}
	}
	return local;
}

inline auto summarize_redif_operator(const RedifOperator &op) -> RedifOperatorDiagnostics
{
	RedifOperatorDiagnostics diagnostics;
	diagnostics.node_count = op.node_count;
	diagnostics.neighbors = op.neighbors;
	diagnostics.reversible = true;
	if (op.node_count == 0) {
		return diagnostics;
	}

	diagnostics.minimum_degree = std::numeric_limits<double>::infinity();
	diagnostics.maximum_degree = 0.0;
	diagnostics.minimum_transition_row_sum = std::numeric_limits<double>::infinity();
	diagnostics.maximum_transition_row_sum = 0.0;
	for (std::size_t i = 0; i < op.node_count; ++i) {
		diagnostics.minimum_degree = std::min(diagnostics.minimum_degree, op.degree[i]);
		diagnostics.maximum_degree = std::max(diagnostics.maximum_degree, op.degree[i]);
		const auto row_sum = op.transition_row_sum(i);
		diagnostics.minimum_transition_row_sum = std::min(diagnostics.minimum_transition_row_sum, row_sum);
		diagnostics.maximum_transition_row_sum = std::max(diagnostics.maximum_transition_row_sum, row_sum);
		for (std::size_t j = i + 1; j < op.node_count; ++j) {
			if (std::abs(op.affinity[i][j] - op.affinity[j][i]) > 1e-12) {
				diagnostics.reversible = false;
			}
		}
	}
	return diagnostics;
}

inline auto redif_operator_from_local_distances(std::vector<std::vector<double>> local_distances,
												std::size_t neighbors) -> RedifOperator
{
	const std::size_t n = local_distances.size();
	std::vector<double> inv_degree(n, 1.0);
	for (std::size_t i = 0; i < n; ++i) {
		double row_sum = 0.0;
		for (std::size_t j = 0; j < n; ++j) {
			row_sum += local_distances[i][j];
		}
		if (row_sum == 0.0) {
			row_sum = 1.0;
		}
		inv_degree[i] = 1.0 / row_sum;
	}

	RedifOperator op;
	op.node_count = n;
	op.neighbors = neighbors;
	op.local_distances = std::move(local_distances);
	op.affinity.assign(n, std::vector<double>(n, 0.0));
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			op.affinity[i][j] = inv_degree[i] * op.local_distances[i][j] * inv_degree[j] /
								 static_cast<double>(n);
		}
	}

	op.degree.assign(n, 0.0);
	op.laplacian.assign(n, std::vector<double>(n, 0.0));
	op.transition.assign(n, std::vector<double>(n, 0.0));
	double degree_total = 0.0;
	for (std::size_t i = 0; i < n; ++i) {
		double weighted_degree = 0.0;
		for (std::size_t j = 0; j < n; ++j) {
			weighted_degree += op.affinity[i][j];
		}
		if (weighted_degree == 0.0) {
			weighted_degree = 1.0 / static_cast<double>(n);
		}
		op.degree[i] = weighted_degree;
		degree_total += weighted_degree;
		op.laplacian[i][i] = weighted_degree;
		for (std::size_t j = 0; j < n; ++j) {
			op.laplacian[i][j] -= op.affinity[i][j];
		}
		if (weighted_degree > 0.0) {
			for (std::size_t j = 0; j < n; ++j) {
				op.transition[i][j] = op.affinity[i][j] / weighted_degree;
			}
		}
		if (op.transition_row_sum(i) == 0.0) {
			op.transition[i][i] = 1.0;
		}
	}

	op.stationary.assign(n, 0.0);
	if (degree_total > 0.0) {
		for (std::size_t i = 0; i < n; ++i) {
			op.stationary[i] = op.degree[i] / degree_total;
		}
	} else if (n > 0) {
		for (std::size_t i = 0; i < n; ++i) {
			op.stationary[i] = 1.0 / static_cast<double>(n);
		}
	}

	op.diagnostics = summarize_redif_operator(op);
	return op;
}

inline auto redif_operator_from_distances(const std::vector<std::vector<double>> &distances, std::size_t neighbors)
	-> RedifOperator
{
	const auto local = local_knn_distance_matrix(distances, neighbors);
	return redif_operator_from_local_distances(local, std::min(neighbors, distances.size() > 0 ? distances.size() - 1 : 0));
}

inline auto solve_linear_system(std::vector<std::vector<double>> matrix, std::vector<double> rhs,
								const std::string &system_name = "Redif linear system")
	-> std::vector<double>
{
	const std::size_t n = matrix.size();
	for (std::size_t pivot = 0; pivot < n; ++pivot) {
		std::size_t best = pivot;
		double best_abs = std::abs(matrix[pivot][pivot]);
		for (std::size_t row = pivot + 1; row < n; ++row) {
			const double candidate = std::abs(matrix[row][pivot]);
			if (candidate > best_abs) {
				best = row;
				best_abs = candidate;
			}
		}
		if (best_abs <= std::numeric_limits<double>::epsilon()) {
			throw std::runtime_error(system_name + " is singular");
		}
		if (best != pivot) {
			std::swap(matrix[pivot], matrix[best]);
			std::swap(rhs[pivot], rhs[best]);
		}

		const double diagonal = matrix[pivot][pivot];
		for (std::size_t col = pivot; col < n; ++col) {
			matrix[pivot][col] /= diagonal;
		}
		rhs[pivot] /= diagonal;

		for (std::size_t row = 0; row < n; ++row) {
			if (row == pivot) {
				continue;
			}
			const double factor = matrix[row][pivot];
			if (factor == 0.0) {
				continue;
			}
			for (std::size_t col = pivot; col < n; ++col) {
				matrix[row][col] -= factor * matrix[pivot][col];
			}
			rhs[row] -= factor * rhs[pivot];
		}
	}
	return rhs;
}

inline auto normalize_probability_row(std::vector<double> &row) -> void
{
	double total = 0.0;
	for (auto &value : row) {
		if (value < 0.0 && value > -1e-12) {
			value = 0.0;
		}
		if (!std::isfinite(value) || value < 0.0) {
			throw std::runtime_error("Redif produced a non-probability measure");
		}
		total += value;
	}
	if (!(total > 0.0)) {
		throw std::runtime_error("Redif produced a zero-mass measure");
	}
	for (auto &value : row) {
		value /= total;
	}
}

inline auto shannon_entropy(const std::vector<double> &measure) -> double
{
	double entropy = 0.0;
	for (const auto mass : measure) {
		if (mass > 0.0) {
			entropy -= mass * std::log(mass);
		}
	}
	return entropy;
}

inline auto relative_entropy(const std::vector<double> &measure, const std::vector<double> &reference) -> double
{
	if (measure.size() != reference.size()) {
		throw std::invalid_argument("relative entropy requires aligned measures");
	}
	double entropy = 0.0;
	for (std::size_t i = 0; i < measure.size(); ++i) {
		if (measure[i] == 0.0) {
			continue;
		}
		if (!(reference[i] > 0.0)) {
			return std::numeric_limits<double>::infinity();
		}
		entropy += measure[i] * std::log(measure[i] / reference[i]);
	}
	return entropy;
}

inline auto validate_redif_state(const RedifOperator &op, const std::vector<std::vector<double>> &state) -> void
{
	if (state.size() != op.node_count) {
		throw std::invalid_argument("Redif state row count must match the operator");
	}
	for (const auto &row : state) {
		if (row.size() != op.node_count) {
			throw std::invalid_argument("Redif state column count must match the operator");
		}
	}
}

inline auto validate_redif_step(double euler_step) -> void
{
	if (!(euler_step > 0.0) || !std::isfinite(euler_step)) {
		throw std::invalid_argument("Redif euler_step must be finite and positive");
	}
	if (euler_step > 1.0) {
		throw std::invalid_argument("Redif euler_step must be <= 1 for probability-measure dynamics");
	}
}

inline auto forward_euler_matrix(const RedifOperator &op, double euler_step) -> std::vector<std::vector<double>>
{
	const std::size_t n = op.node_count;
	std::vector<std::vector<double>> matrix(n, std::vector<double>(n, 0.0));
	for (std::size_t i = 0; i < n; ++i) {
		matrix[i][i] = 1.0 - euler_step;
		for (std::size_t j = 0; j < n; ++j) {
			matrix[i][j] += euler_step * op.transition[i][j];
		}
	}
	return matrix;
}

inline auto transpose(const std::vector<std::vector<double>> &matrix) -> std::vector<std::vector<double>>
{
	const std::size_t n = matrix.size();
	std::vector<std::vector<double>> result(n, std::vector<double>(n, 0.0));
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			result[j][i] = matrix[i][j];
		}
	}
	return result;
}

inline auto redif_implicit_euler_step(const RedifOperator &op, const std::vector<std::vector<double>> &state,
									  double euler_step) -> std::vector<std::vector<double>>
{
	validate_redif_state(op, state);
	validate_redif_step(euler_step);
	const std::size_t n = op.node_count;
	std::vector<std::vector<double>> left(n, std::vector<double>(n, 0.0));
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			left[i][j] = euler_step * op.laplacian[i][j];
		}
		left[i][i] += op.degree[i];
	}

	std::vector<std::vector<double>> next(n, std::vector<double>(n, 0.0));
	for (std::size_t coordinate = 0; coordinate < n; ++coordinate) {
		std::vector<double> rhs(n, 0.0);
		for (std::size_t row = 0; row < n; ++row) {
			rhs[row] = op.degree[row] * state[row][coordinate];
		}
		auto solution = solve_linear_system(left, std::move(rhs), "Redif implicit Euler system");
		for (std::size_t row = 0; row < n; ++row) {
			next[row][coordinate] = solution[row];
		}
	}

	for (auto &row : next) {
		normalize_probability_row(row);
	}
	return next;
}

inline auto redif_forward_euler_step(const RedifOperator &op, const std::vector<std::vector<double>> &state,
									 double euler_step) -> std::vector<std::vector<double>>
{
	validate_redif_state(op, state);
	validate_redif_step(euler_step);
	const std::size_t n = op.node_count;
	const auto matrix = forward_euler_matrix(op, euler_step);
	std::vector<std::vector<double>> next(n, std::vector<double>(n, 0.0));
	for (std::size_t row = 0; row < n; ++row) {
		for (std::size_t via = 0; via < n; ++via) {
			const double mass = state[row][via];
			if (mass == 0.0) {
				continue;
			}
			for (std::size_t col = 0; col < n; ++col) {
				next[row][col] += mass * matrix[via][col];
			}
		}
	}
	for (auto &row : next) {
		normalize_probability_row(row);
	}
	return next;
}

inline auto redif_inverse_forward_euler_step(const RedifOperator &op,
											 const std::vector<std::vector<double>> &state,
											 double euler_step) -> std::vector<std::vector<double>>
{
	validate_redif_state(op, state);
	validate_redif_step(euler_step);
	const std::size_t n = op.node_count;
	const auto left = transpose(forward_euler_matrix(op, euler_step));
	std::vector<std::vector<double>> previous(n, std::vector<double>(n, 0.0));
	for (std::size_t row = 0; row < n; ++row) {
		auto solution = solve_linear_system(left, state[row], "Redif inverse forward Euler system");
		normalize_probability_row(solution);
		previous[row] = std::move(solution);
	}
	return previous;
}

inline auto transport_distance_matrix(const std::vector<std::vector<double>> &state,
									  const Wasserstein<double> &transport)
	-> std::vector<std::vector<double>>
{
	const std::size_t n = state.size();
	std::vector<std::vector<double>> distances(n, std::vector<double>(n, 0.0));
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = i + 1; j < n; ++j) {
			const double value = transport(state[i], state[j]);
			distances[i][j] = value;
			distances[j][i] = value;
		}
	}
	return distances;
}

inline auto transition_step_transport(const std::vector<std::vector<double>> &state,
									  const std::vector<std::vector<double>> &next,
									  const Wasserstein<double> &transport)
	-> std::vector<double>
{
	std::vector<double> step(state.size(), 0.0);
	for (std::size_t record = 0; record < state.size(); ++record) {
		step[record] = transport(state[record], next[record]);
	}
	return step;
}

inline auto make_measure_result(const std::vector<RecordId> &ids,
								const std::vector<std::vector<std::vector<double>>> &frames,
								const std::vector<std::vector<double>> &step_transport,
								std::vector<double> initial_stationary,
								std::vector<RedifOperatorDiagnostics> operator_diagnostics,
								std::size_t neighbors, const redif_options &options,
								std::string operator_name, std::string strategy,
								std::string representation) -> RedifMeasureResult
{
	const std::size_t n = ids.size();
	std::vector<RedifMeasurePath> paths;
	paths.reserve(n);
	for (std::size_t record = 0; record < n; ++record) {
		RedifMeasurePath path;
		path.id = ids[record];
		path.measures.reserve(frames.size());
		path.step_transport.reserve(step_transport.size());
		for (const auto &frame : frames) {
			path.measures.push_back(frame[record]);
		}
		for (const auto &step : step_transport) {
			path.step_transport.push_back(step[record]);
			path.transport_path_length += step[record];
		}
		paths.push_back(std::move(path));
	}

	std::vector<RedifEntropyDiagnostics> entropy_diagnostics;
	if (!initial_stationary.empty()) {
		entropy_diagnostics.reserve(paths.size());
		for (const auto &path : paths) {
			entropy_diagnostics.push_back(RedifEntropyDiagnostics{
				path.id,
				shannon_entropy(path.measures.front()),
				shannon_entropy(path.terminal_measure()),
				relative_entropy(path.measures.front(), initial_stationary),
				relative_entropy(path.terminal_measure(), initial_stationary)});
		}
	}

	return RedifMeasureResult{std::move(paths),
							  n,
							  neighbors,
							  options.iterations,
							  options.euler_step,
							  options.adaptive_geometry,
							  true,
							  std::move(operator_name),
							  std::move(strategy),
							  std::move(representation),
							  std::move(initial_stationary),
							  std::move(entropy_diagnostics),
							  std::move(operator_diagnostics)};
}

} // namespace detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto redif_operator(const Space &space, redif_options options = {}) -> RedifOperator
{
	detail::validate_redif_options(space.size(), options);
	const std::size_t k = std::min(options.neighbors, space.size() > 0 ? space.size() - 1 : 0);
	return detail::redif_operator_from_distances(detail::distance_matrix(space), k);
}

inline auto redif_forward_noise_step(const RedifOperator &op, const std::vector<std::vector<double>> &state,
									 double euler_step) -> std::vector<std::vector<double>>
{
	return detail::redif_forward_euler_step(op, state, euler_step);
}

inline auto redif_inverse_noise_step(const RedifOperator &op, const std::vector<std::vector<double>> &state,
									 double euler_step) -> std::vector<std::vector<double>>
{
	return detail::redif_inverse_forward_euler_step(op, state, euler_step);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto redif_remove_noise(const Space &space, redif_options options = {}) -> RedifMeasureResult
{
	detail::validate_redif_options(space.size(), options);
	const std::size_t n = space.size();
	const std::size_t k = std::min(options.neighbors, n > 0 ? n - 1 : 0);

	const auto source_distances = detail::distance_matrix(space);
	Wasserstein<double> transport(source_distances);
	std::vector<RecordId> ids;
	ids.reserve(n);
	for (std::size_t i = 0; i < n; ++i) {
		ids.push_back(space.id(i));
	}

	std::vector<std::vector<std::vector<double>>> frames;
	std::vector<std::vector<double>> step_transport;
	std::vector<RedifOperatorDiagnostics> operator_diagnostics;
	std::vector<double> initial_stationary;
	frames.reserve(options.iterations + 1);
	step_transport.reserve(options.iterations);
	operator_diagnostics.reserve(options.iterations);

	auto state = detail::identity_measures(n);
	auto geometry_distances = source_distances;
	frames.push_back(state);

	for (std::size_t iteration = 0; iteration < options.iterations; ++iteration) {
		const auto op = detail::redif_operator_from_distances(geometry_distances, k);
		if (initial_stationary.empty()) {
			initial_stationary = op.stationary;
		}
		operator_diagnostics.push_back(op.diagnostics);
		const auto next = detail::redif_implicit_euler_step(op, state, options.euler_step);

		std::vector<double> step(n, 0.0);
		for (std::size_t record = 0; record < n; ++record) {
			step[record] = transport(state[record], next[record]);
		}
		step_transport.push_back(std::move(step));
		state = next;
		frames.push_back(state);
		if (options.adaptive_geometry) {
			geometry_distances = detail::transport_distance_matrix(state, transport);
		}
	}

	return detail::make_measure_result(ids, frames, step_transport, std::move(initial_stationary),
									   std::move(operator_diagnostics), k, options, "redif_remove_noise",
									   options.adaptive_geometry ? "redif_inverse_implicit_euler_adaptive"
																 : "redif_inverse_implicit_euler_fixed",
									   "transport_measure_path");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto redif_add_noise(const Space &space, redif_options options = {}) -> RedifMeasureResult
{
	detail::validate_redif_options(space.size(), options);
	const std::size_t n = space.size();
	const std::size_t k = std::min(options.neighbors, n > 0 ? n - 1 : 0);
	const auto source_distances = detail::distance_matrix(space);
	Wasserstein<double> transport(source_distances);
	std::vector<RecordId> ids;
	ids.reserve(n);
	for (std::size_t i = 0; i < n; ++i) {
		ids.push_back(space.id(i));
	}

	std::vector<std::vector<std::vector<double>>> frames;
	std::vector<std::vector<double>> step_transport;
	std::vector<RedifOperatorDiagnostics> operator_diagnostics;
	std::vector<double> initial_stationary;
	frames.reserve(options.iterations + 1);
	step_transport.reserve(options.iterations);
	operator_diagnostics.reserve(options.iterations);
	auto state = detail::identity_measures(n);
	auto geometry_distances = source_distances;
	frames.push_back(state);
	for (std::size_t iteration = 0; iteration < options.iterations; ++iteration) {
		const auto op = detail::redif_operator_from_distances(geometry_distances, k);
		if (initial_stationary.empty()) {
			initial_stationary = op.stationary;
		}
		operator_diagnostics.push_back(op.diagnostics);
		auto next = detail::redif_forward_euler_step(op, state, options.euler_step);
		step_transport.push_back(detail::transition_step_transport(state, next, transport));
		state = std::move(next);
		frames.push_back(state);
		if (options.adaptive_geometry) {
			geometry_distances = detail::transport_distance_matrix(state, transport);
		}
	}

	return detail::make_measure_result(ids, frames, step_transport, std::move(initial_stationary),
									   std::move(operator_diagnostics), k, options,
									   "redif_add_noise",
									   options.adaptive_geometry ? "redif_forward_euler_adaptive"
																 : "redif_forward_euler_fixed",
									   "transport_measure_path");
}

inline auto entropy_trajectory(const RedifMeasurePath &path, const std::vector<double> &stationary)
	-> EntropyTrajectory
{
	EntropyTrajectory result;
	result.shannon_entropy.reserve(path.measures.size());
	result.relative_entropy_to_stationary.reserve(path.measures.size());
	for (const auto &measure : path.measures) {
		result.shannon_entropy.push_back(detail::shannon_entropy(measure));
		result.relative_entropy_to_stationary.push_back(detail::relative_entropy(measure, stationary));
	}
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto redif_outliers(const Space &space, redif_options options = {}) -> OutlierResult<double>
{
	const auto paths = redif_remove_noise(space, std::move(options));
	std::vector<Outlier<double>> outliers;
	outliers.reserve(paths.size());
	for (const auto &path : paths.paths) {
		outliers.push_back(Outlier<double>{path.id, path.transport_path_length});
	}
	core::sort_outliers(outliers);
	return core::make_outlier_result(std::move(outliers), paths.record_count, 0, 0, "redif_transport_path_length",
									 paths.representation, paths.exact);
}

} // namespace mtrc::modify::dynamics

namespace mtrc {
using RedifOperator = modify::dynamics::RedifOperator;
using RedifOperatorDiagnostics = modify::dynamics::RedifOperatorDiagnostics;
using RedifEntropyDiagnostics = modify::dynamics::RedifEntropyDiagnostics;
using RedifMeasurePath = modify::dynamics::RedifMeasurePath;
using RedifMeasureResult = modify::dynamics::RedifMeasureResult;
using EntropyTrajectory = modify::dynamics::EntropyTrajectory;
using redif_options = modify::dynamics::redif_options;
using modify::dynamics::entropy_trajectory;
using modify::dynamics::redif_add_noise;
using modify::dynamics::redif_forward_noise_step;
using modify::dynamics::redif_inverse_noise_step;
using modify::dynamics::redif_operator;
using modify::dynamics::redif_outliers;
using modify::dynamics::redif_remove_noise;
} // namespace mtrc

#endif
