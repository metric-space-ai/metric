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

enum class redif_scale_policy {
	mean_local_distance,
	median_local_distance,
	kth_local_distance,
	global_mean_distance
};

inline auto redif_scale_policy_name(redif_scale_policy policy) -> std::string
{
	switch (policy) {
	case redif_scale_policy::mean_local_distance:
		return "mean_local_distance";
	case redif_scale_policy::median_local_distance:
		return "median_local_distance";
	case redif_scale_policy::kth_local_distance:
		return "kth_local_distance";
	case redif_scale_policy::global_mean_distance:
		return "global_mean_distance";
	}
	return "unknown_local_scale_policy";
}

struct redif_options {
	std::size_t neighbors = 10;
	std::size_t iterations = 15;
	double euler_step = 0.25;
	bool adaptive_geometry = true;
	redif_scale_policy scale_policy = redif_scale_policy::mean_local_distance;
	double stability_tolerance = 1.0e-12;
	double marginal_stability_tolerance = 1.0e-8;
	std::size_t max_dense_records = default_metric_transition_max_dense_records;
	std::size_t max_memory_bytes = default_metric_transition_max_memory_bytes;
	std::size_t max_distance_evaluations = default_metric_transition_max_distance_evaluations;
	std::size_t max_transport_problems = 0;
	std::size_t max_transport_support_atoms = 0;
	double transport_support_mass_floor = 0.0;
	bool allow_transport_support_truncation = false;
};

struct redif_sampled_relation_options {
	std::size_t candidate_count{};
	std::size_t chunk_size{};
	std::size_t offset = 1;
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
	std::string local_relation_representation{"dense_distance_matrix"};
	std::string local_relation_exactness{"exact"};
	std::size_t local_relation_directed_entries{};
	std::size_t local_relation_distance_evaluations{};
	std::size_t dense_distance_evaluations{};
	std::size_t local_relation_candidate_count{};
	std::size_t local_relation_candidate_universe{};
	std::size_t local_relation_chunk_size{};
	std::size_t local_relation_chunk_count{};
	double local_relation_candidate_fraction{1.0};
	bool local_relation_exact{true};
	bool local_relation_refused{false};
	std::string local_relation_refusal_reason;
	double minimum_degree{};
	double maximum_degree{};
	double degree_ratio{};
	double minimum_local_scale{};
	double maximum_local_scale{};
	double minimum_positive_affinity{};
	double maximum_affinity{};
	double minimum_transition_row_sum{};
	double maximum_transition_row_sum{};
	double maximum_self_transition_probability{};
	double minimum_transition_escape_probability{};
	double spectral_gap_proxy_value{};
	std::string spectral_gap_proxy{"minimum_transition_escape_probability"};
	std::size_t component_count{};
	bool reversible{true};
};

struct RedifStabilityDiagnostics {
	std::string status{"not_evaluated"};
	double minimum_pivot_abs{};
	double maximum_pivot_abs{};
	double pivot_ratio{};
	double singularity_margin{};
	bool stable{true};
	bool marginal{false};
};

struct RedifStepDiagnostics {
	std::size_t step{};
	RedifOperatorDiagnostics operator_diagnostics;
	RedifStabilityDiagnostics stability;
	std::vector<double> stationary;
	double minimum_shannon_entropy{};
	double maximum_shannon_entropy{};
	double maximum_relative_entropy_to_current_stationary{};
};

struct RedifTransportDiagnostics {
	std::string ground_metric{"source_metric"};
	std::string solver{"exact_discrete_wasserstein"};
	std::size_t measure_count{};
	std::size_t pair_count{};
	std::size_t transport_problem_count{};
	std::size_t max_transport_problems{};
	std::size_t support_atom_count{};
	std::size_t max_transport_support_atoms{};
	std::size_t truncated_measure_count{};
	double discarded_mass_total{};
	double maximum_discarded_mass{};
	double support_mass_floor{};
	bool exact{true};
	bool support_truncated{false};
	bool refused{false};
	std::string exactness{"exact"};
	std::string refusal_reason;
};

struct RedifLocalRelationEntry {
	std::size_t source{};
	std::size_t target{};
	double distance{};
};

struct RedifSparseEntry {
	std::size_t index{};
	double distance{};
	double affinity{};
	double transition{};
};

struct RedifLocalRelationDiagnostics {
	std::size_t node_count{};
	std::size_t neighbors{};
	std::string representation{"dense_distance_matrix"};
	std::string exactness{"exact"};
	std::size_t directed_entries{};
	std::size_t symmetric_entries{};
	std::size_t distance_evaluations{};
	std::size_t candidate_count{};
	std::size_t candidate_universe{};
	std::size_t chunk_size{};
	std::size_t chunk_count{};
	double candidate_fraction{1.0};
	bool exact{true};
	bool refused{false};
	std::string refusal_reason;
};

struct RedifSparseOperator {
	std::size_t node_count{};
	std::size_t neighbors{};
	std::string affinity_kernel{"redif_self_tuned_heat_kernel"};
	std::string local_scale_policy{"mean_local_distance"};
	std::string generator{"degree_weighted_laplacian_resolvent"};
	std::vector<std::vector<RedifSparseEntry>> rows;
	std::vector<double> local_scale;
	std::vector<double> degree;
	std::vector<double> stationary;
	RedifLocalRelationDiagnostics local_relation_diagnostics;
	RedifOperatorDiagnostics diagnostics;

	auto size() const -> std::size_t { return node_count; }

	auto transition_row_sum(std::size_t row) const -> double
	{
		double total = 0.0;
		for (const auto &entry : rows[row]) {
			total += entry.transition;
		}
		return total;
	}
};

struct RedifOperator {
	std::size_t node_count{};
	std::size_t neighbors{};
	std::string affinity_kernel{"redif_self_tuned_heat_kernel"};
	std::string local_scale_policy{"mean_local_distance"};
	std::string generator{"degree_weighted_laplacian_resolvent"};
	std::vector<std::vector<double>> local_distances;
	std::vector<std::vector<unsigned char>> local_relation;
	std::vector<double> local_scale;
	std::vector<std::vector<double>> affinity;
	std::vector<double> degree;
	std::vector<std::vector<double>> laplacian;
	std::vector<std::vector<double>> transition;
	std::vector<std::vector<RedifSparseEntry>> sparse_rows;
	std::vector<double> stationary;
	RedifLocalRelationDiagnostics local_relation_diagnostics;
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
	double terminal_relative_entropy_to_terminal_stationary{};
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

struct RedifMeasureSummary {
	RecordId id;
	double transport_path_length{};
	double initial_shannon_entropy{};
	double terminal_shannon_entropy{};
	double entropy_delta{};
	std::size_t terminal_top_record_index{};
	double terminal_top_record_mass{};
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
	std::vector<double> terminal_stationary;
	std::vector<RedifEntropyDiagnostics> entropy_diagnostics;
	std::vector<RedifOperatorDiagnostics> operator_diagnostics;
	std::vector<RedifStepDiagnostics> step_diagnostics;
	RedifTransportDiagnostics transport_diagnostics;

	auto size() const -> std::size_t { return paths.size(); }
	auto empty() const -> bool { return paths.empty(); }
	auto begin() const -> std::vector<RedifMeasurePath>::const_iterator { return paths.begin(); }
	auto end() const -> std::vector<RedifMeasurePath>::const_iterator { return paths.end(); }
	auto operator[](std::size_t index) const -> const RedifMeasurePath & { return paths[index]; }

	auto summaries() const -> std::vector<RedifMeasureSummary>;
};

struct RedifMultiscaleScore {
	RecordId id;
	std::vector<double> scale_path_lengths;
	std::vector<double> scale_stability_weights;
	double median_path_length{};
	double maximum_path_length{};
	double stability_weighted_path_length{};
};

struct RedifScaleConfiguration {
	redif_options options;
	std::string label;
};

struct RedifMultiscaleResult {
	std::vector<RedifMultiscaleScore> scores;
	std::vector<std::string> scale_labels;
	std::string aggregate{"median_transport_path_length"};
	bool exact{true};
};

namespace detail {

struct RedifLocalRelation {
	std::vector<std::vector<double>> distances;
	std::vector<std::vector<unsigned char>> connected;
};

inline auto redif_transport_problem_count(std::size_t record_count, std::size_t iterations) -> std::size_t
{
	return checked_dense_product(record_count, iterations, "Redif transport path");
}

inline auto require_redif_transport_budget(std::size_t record_count, const redif_options &options) -> void
{
	if (options.max_transport_problems > 0) {
		const auto problem_count = redif_transport_problem_count(record_count, options.iterations);
		if (problem_count > options.max_transport_problems) {
			throw MetricInputError("Redif transport path exceeds max_transport_problems: records=" +
								   std::to_string(record_count) + " iterations=" +
								   std::to_string(options.iterations) + " estimated_transport_problems=" +
								   std::to_string(problem_count) + " max_transport_problems=" +
								   std::to_string(options.max_transport_problems));
		}
	}
	if (options.max_transport_support_atoms > 0 && options.max_transport_support_atoms < record_count &&
		!options.allow_transport_support_truncation) {
		throw MetricInputError(
			"Redif exact transport requires full measure support: records=" + std::to_string(record_count) +
			" max_transport_support_atoms=" + std::to_string(options.max_transport_support_atoms) +
			". Enable allow_transport_support_truncation only when a non-exact transport path is acceptable.");
	}
	if (options.transport_support_mass_floor > 0.0 && !options.allow_transport_support_truncation) {
		throw MetricInputError(
			"Redif transport_support_mass_floor discards measure mass and requires allow_transport_support_truncation");
	}
}

inline auto validate_redif_options(std::size_t record_count, const redif_options &options,
								   bool require_dense_construction = true) -> void
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
	if (!(options.stability_tolerance > 0.0) || !std::isfinite(options.stability_tolerance)) {
		throw std::invalid_argument("Redif stability_tolerance must be finite and positive");
	}
	if (!(options.marginal_stability_tolerance >= options.stability_tolerance) ||
		!std::isfinite(options.marginal_stability_tolerance)) {
		throw std::invalid_argument("Redif marginal_stability_tolerance must be finite and >= stability_tolerance");
	}
	if (options.max_transport_support_atoms > record_count) {
		throw std::invalid_argument("Redif max_transport_support_atoms cannot exceed the record count");
	}
	if (options.transport_support_mass_floor < 0.0 || !std::isfinite(options.transport_support_mass_floor)) {
		throw std::invalid_argument("Redif transport_support_mass_floor must be finite and nonnegative");
	}
	require_redif_transport_budget(record_count, options);
	if (require_dense_construction) {
		require_dense_metric_budget("Redif", record_count, options.max_dense_records,
									options.max_memory_bytes, options.max_distance_evaluations,
									options.iterations + 8);
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

inline auto validate_redif_distance_matrix(const std::vector<std::vector<double>> &distances) -> void
{
	const std::size_t n = distances.size();
	for (std::size_t i = 0; i < n; ++i) {
		if (distances[i].size() != n) {
			throw std::invalid_argument("Redif distance matrix must be square");
		}
		for (std::size_t j = 0; j < n; ++j) {
			const auto value = distances[i][j];
			if (!std::isfinite(value) || value < 0.0) {
				throw std::invalid_argument("Redif distance matrix entries must be finite and nonnegative");
			}
			if (i == j && std::abs(value) > 1e-12) {
				throw std::invalid_argument("Redif distance matrix diagonal must be zero");
			}
			if (j < i && std::abs(value - distances[j][i]) > 1e-9) {
				throw std::invalid_argument("Redif distance matrix must be symmetric");
			}
		}
	}
}

inline auto local_knn_relation(const std::vector<std::vector<double>> &distances, std::size_t neighbors)
	-> RedifLocalRelation
{
	validate_redif_distance_matrix(distances);
	const std::size_t n = distances.size();
	const std::size_t k = std::min(neighbors, n > 0 ? n - 1 : 0);
	std::vector<std::vector<double>> local(n, std::vector<double>(n, 0.0));
	std::vector<std::vector<unsigned char>> connected(n, std::vector<unsigned char>(n, 0));

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
			connected[i][order[t]] = 1;
		}
	}

	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = i + 1; j < n; ++j) {
			if (connected[i][j] || connected[j][i]) {
				local[i][j] = distances[i][j];
				local[j][i] = distances[i][j];
				connected[i][j] = 1;
				connected[j][i] = 1;
			}
		}
	}
	return RedifLocalRelation{std::move(local), std::move(connected)};
}

inline auto local_knn_distance_matrix(const std::vector<std::vector<double>> &distances, std::size_t neighbors)
	-> std::vector<std::vector<double>>
{
	return local_knn_relation(distances, neighbors).distances;
}

inline auto redif_directed_relation_entry_count(const std::vector<std::vector<unsigned char>> &connected)
	-> std::size_t
{
	std::size_t count = 0;
	for (const auto &row : connected) {
		for (const auto value : row) {
			if (value != 0) {
				++count;
			}
		}
	}
	return count;
}

inline auto positive_or_default(std::vector<double> values, double fallback) -> double
{
	values.erase(std::remove_if(values.begin(), values.end(), [](double value) { return !(value > 0.0); }),
				 values.end());
	if (values.empty()) {
		return fallback > 0.0 ? fallback : 1.0;
	}
	std::sort(values.begin(), values.end());
	return values[values.size() / 2];
}

inline auto global_positive_scale(const RedifLocalRelation &local) -> double
{
	double total = 0.0;
	std::size_t count = 0;
	for (std::size_t i = 0; i < local.distances.size(); ++i) {
		for (std::size_t j = i + 1; j < local.distances.size(); ++j) {
			if (local.connected[i][j] && local.distances[i][j] > 0.0) {
				total += local.distances[i][j];
				++count;
			}
		}
	}
	return count > 0 ? total / static_cast<double>(count) : 1.0;
}

inline auto redif_local_scales(const RedifLocalRelation &local, redif_scale_policy policy) -> std::vector<double>
{
	const std::size_t n = local.distances.size();
	std::vector<double> scales(n, global_positive_scale(local));
	const auto global_scale = scales.empty() ? 1.0 : scales.front();
	if (policy == redif_scale_policy::global_mean_distance) {
		return scales;
	}
	for (std::size_t i = 0; i < n; ++i) {
		double total = 0.0;
		std::size_t count = 0;
		std::vector<double> positives;
		positives.reserve(n);
		for (std::size_t j = 0; j < n; ++j) {
			if (!local.connected[i][j]) {
				continue;
			}
			const auto distance = local.distances[i][j];
			if (distance > 0.0) {
				total += distance;
				++count;
				positives.push_back(distance);
			}
		}
		if (policy == redif_scale_policy::mean_local_distance && count > 0) {
			scales[i] = total / static_cast<double>(count);
		} else if (policy == redif_scale_policy::median_local_distance) {
			scales[i] = positive_or_default(std::move(positives), global_scale);
		} else if (policy == redif_scale_policy::kth_local_distance && !positives.empty()) {
			scales[i] = *std::max_element(positives.begin(), positives.end());
		}
	}
	return scales;
}

inline auto redif_heat_affinity(bool connected, double distance, double source_scale, double target_scale) -> double
{
	if (!connected) {
		return 0.0;
	}
	if (!(source_scale > 0.0) || !(target_scale > 0.0) || !std::isfinite(source_scale) ||
		!std::isfinite(target_scale)) {
		throw std::invalid_argument("Redif local scales must be finite and positive");
	}
	if (distance == 0.0) {
		return 1.0;
	}
	const auto scale_product = source_scale * target_scale;
	return std::exp(-(distance * distance) / scale_product);
}

inline auto validate_redif_relation_entry(std::size_t record_count, const RedifLocalRelationEntry &entry) -> void
{
	if (entry.source >= record_count || entry.target >= record_count) {
		throw std::invalid_argument("Redif local relation entry index is outside the finite space");
	}
	if (entry.source == entry.target) {
		throw std::invalid_argument("Redif local relation entries must not contain self entries");
	}
	if (!std::isfinite(entry.distance) || entry.distance < 0.0) {
		throw std::invalid_argument("Redif local relation distances must be finite and nonnegative");
	}
}

inline auto insert_sparse_relation_distance(std::vector<std::vector<std::pair<std::size_t, double>>> &rows,
											std::size_t source, std::size_t target, double distance,
											bool allow_existing) -> bool
{
	auto &row = rows[source];
	for (auto &entry : row) {
		if (entry.first == target) {
			if (std::abs(entry.second - distance) > 1e-9) {
				throw std::invalid_argument("Redif exact local relation contains inconsistent symmetric distances");
			}
			if (!allow_existing) {
				throw std::invalid_argument("Redif exact local relation contains duplicate directed entries");
			}
			return false;
		}
	}
	row.push_back({target, distance});
	return true;
}

inline auto local_relation_from_sparse_distance_rows(
	const std::vector<std::vector<std::pair<std::size_t, double>>> &rows) -> RedifLocalRelation
{
	const std::size_t n = rows.size();
	RedifLocalRelation local;
	local.distances.assign(n, std::vector<double>(n, 0.0));
	local.connected.assign(n, std::vector<unsigned char>(n, 0));
	for (std::size_t i = 0; i < n; ++i) {
		for (const auto &entry : rows[i]) {
			local.distances[i][entry.first] = entry.second;
			local.connected[i][entry.first] = 1;
		}
	}
	return local;
}

inline auto redif_sampled_candidate_order(std::size_t record_count, std::size_t source,
										  redif_sampled_relation_options sample_options)
	-> std::vector<std::size_t>
{
	std::vector<std::size_t> order;
	order.reserve(record_count > 0 ? record_count - 1 : 0);
	if (record_count <= 1) {
		return order;
	}
	if (sample_options.chunk_size > 0) {
		const auto chunk_size = std::min(sample_options.chunk_size, record_count);
		const auto chunk_count = (record_count + chunk_size - 1) / chunk_size;
		const auto source_chunk = source / chunk_size;
		for (std::size_t chunk_delta = 0; chunk_delta < chunk_count; ++chunk_delta) {
			const auto chunk = (source_chunk + chunk_delta) % chunk_count;
			const auto begin = chunk * chunk_size;
			const auto end = std::min(record_count, begin + chunk_size);
			for (std::size_t index = begin; index < end; ++index) {
				if (index != source) {
					order.push_back(index);
				}
			}
		}
		return order;
	}
	auto step = sample_options.offset == 0 ? std::size_t{1} : sample_options.offset;
	while (order.size() < record_count - 1) {
		const auto candidate = (source + step) % record_count;
		if (candidate != source &&
			std::find(order.begin(), order.end(), candidate) == order.end()) {
			order.push_back(candidate);
		}
		++step;
	}
	return order;
}

inline auto sampled_chunked_local_relation(const std::vector<std::vector<double>> &distances,
										   std::size_t neighbors,
										   redif_sampled_relation_options sample_options,
										   RedifLocalRelationDiagnostics *diagnostics = nullptr)
	-> RedifLocalRelation
{
	validate_redif_distance_matrix(distances);
	const std::size_t n = distances.size();
	const std::size_t k = std::min(neighbors, n > 0 ? n - 1 : 0);
	if (sample_options.candidate_count == 0 && n > 1) {
		throw std::invalid_argument("Redif sampled relation candidate_count must be positive");
	}
	const auto candidate_universe = n > 0 ? n - 1 : 0;
	const auto candidate_count = std::min(sample_options.candidate_count, candidate_universe);
	if (candidate_count < k) {
		throw std::invalid_argument("Redif sampled relation candidate_count must be >= neighbors");
	}
	std::vector<std::vector<std::pair<std::size_t, double>>> rows(n);
	std::size_t directed_entries = 0;
	for (std::size_t i = 0; i < n; ++i) {
		auto candidates = redif_sampled_candidate_order(n, i, sample_options);
		if (candidates.size() > candidate_count) {
			candidates.resize(candidate_count);
		}
		std::sort(candidates.begin(), candidates.end(), [&](std::size_t lhs, std::size_t rhs) {
			if (distances[i][lhs] != distances[i][rhs]) {
				return distances[i][lhs] < distances[i][rhs];
			}
			return lhs < rhs;
		});
		for (std::size_t t = 0; t < k; ++t) {
			const auto target = candidates[t];
			if (insert_sparse_relation_distance(rows, i, target, distances[i][target], false)) {
				++directed_entries;
			}
		}
	}
	std::vector<RedifLocalRelationEntry> directed;
	directed.reserve(directed_entries);
	for (std::size_t source = 0; source < rows.size(); ++source) {
		for (const auto &entry : rows[source]) {
			directed.push_back(RedifLocalRelationEntry{source, entry.first, entry.second});
		}
	}
	for (const auto &entry : directed) {
		(void)insert_sparse_relation_distance(rows, entry.target, entry.source, entry.distance, true);
	}
	for (auto &row : rows) {
		std::sort(row.begin(), row.end(), [](const auto &lhs, const auto &rhs) {
			if (lhs.first != rhs.first) {
				return lhs.first < rhs.first;
			}
			return lhs.second < rhs.second;
		});
	}

	if (diagnostics != nullptr) {
		diagnostics->node_count = n;
		diagnostics->neighbors = k;
		diagnostics->representation =
			sample_options.chunk_size > 0 ? "sampled_chunked_local_relation" : "sampled_local_relation";
		diagnostics->exact = candidate_count == candidate_universe;
		diagnostics->exactness = diagnostics->exact ? "exact" : "non_exact_sampled_local_relation";
		diagnostics->directed_entries = directed_entries;
		diagnostics->symmetric_entries = 0;
		for (const auto &row : rows) {
			diagnostics->symmetric_entries += row.size();
		}
		diagnostics->distance_evaluations =
			checked_dense_product(n, candidate_count, "Redif sampled local relation");
		diagnostics->candidate_count = candidate_count;
		diagnostics->candidate_universe = candidate_universe;
		diagnostics->chunk_size = sample_options.chunk_size;
		diagnostics->chunk_count =
			sample_options.chunk_size > 0 ? (n + sample_options.chunk_size - 1) / sample_options.chunk_size : 1;
		diagnostics->candidate_fraction =
			candidate_universe > 0 ? static_cast<double>(candidate_count) / static_cast<double>(candidate_universe)
								   : 1.0;
		diagnostics->refused = false;
		diagnostics->refusal_reason.clear();
	}
	return local_relation_from_sparse_distance_rows(rows);
}

inline auto sparse_relation_from_exact_entries(std::size_t record_count, std::size_t neighbors,
											   const std::vector<RedifLocalRelationEntry> &entries,
											   RedifLocalRelationDiagnostics *diagnostics = nullptr)
	-> std::vector<std::vector<std::pair<std::size_t, double>>>
{
	const std::size_t k = std::min(neighbors, record_count > 0 ? record_count - 1 : 0);
	std::vector<std::vector<std::pair<std::size_t, double>>> rows(record_count);
	std::vector<std::size_t> directed_counts(record_count, 0);

	for (const auto &entry : entries) {
		validate_redif_relation_entry(record_count, entry);
		if (insert_sparse_relation_distance(rows, entry.source, entry.target, entry.distance, false)) {
			++directed_counts[entry.source];
		}
	}
	for (std::size_t row = 0; row < record_count; ++row) {
		if (directed_counts[row] != k) {
			throw std::invalid_argument(
				"Redif exact local relation requires exactly k directed entries per atom");
		}
	}

	std::vector<RedifLocalRelationEntry> directed;
	directed.reserve(entries.size());
	for (std::size_t source = 0; source < rows.size(); ++source) {
		for (const auto &entry : rows[source]) {
			directed.push_back(RedifLocalRelationEntry{source, entry.first, entry.second});
		}
	}
	for (const auto &entry : directed) {
		(void)insert_sparse_relation_distance(rows, entry.target, entry.source, entry.distance, true);
	}
	for (auto &row : rows) {
		std::sort(row.begin(), row.end(), [](const auto &lhs, const auto &rhs) {
			if (lhs.first != rhs.first) {
				return lhs.first < rhs.first;
			}
			return lhs.second < rhs.second;
		});
	}

	if (diagnostics != nullptr) {
		diagnostics->node_count = record_count;
		diagnostics->neighbors = k;
		diagnostics->representation = "exact_neighbor_provider";
		diagnostics->exactness = "exact";
		diagnostics->directed_entries = entries.size();
		diagnostics->symmetric_entries = 0;
		for (const auto &row : rows) {
			diagnostics->symmetric_entries += row.size();
		}
		diagnostics->distance_evaluations = entries.size();
		diagnostics->exact = true;
		diagnostics->refused = false;
		diagnostics->refusal_reason.clear();
	}
	return rows;
}

inline auto redif_sparse_global_positive_scale(
	const std::vector<std::vector<std::pair<std::size_t, double>>> &rows) -> double
{
	double total = 0.0;
	std::size_t count = 0;
	for (std::size_t i = 0; i < rows.size(); ++i) {
		for (const auto &entry : rows[i]) {
			if (entry.first > i && entry.second > 0.0) {
				total += entry.second;
				++count;
			}
		}
	}
	return count > 0 ? total / static_cast<double>(count) : 1.0;
}

inline auto redif_sparse_local_scales(const std::vector<std::vector<std::pair<std::size_t, double>>> &rows,
									  redif_scale_policy policy) -> std::vector<double>
{
	std::vector<double> scales(rows.size(), redif_sparse_global_positive_scale(rows));
	const auto global_scale = scales.empty() ? 1.0 : scales.front();
	if (policy == redif_scale_policy::global_mean_distance) {
		return scales;
	}
	for (std::size_t i = 0; i < rows.size(); ++i) {
		double total = 0.0;
		std::size_t count = 0;
		std::vector<double> positives;
		positives.reserve(rows[i].size());
		for (const auto &entry : rows[i]) {
			if (entry.second > 0.0) {
				total += entry.second;
				++count;
				positives.push_back(entry.second);
			}
		}
		if (policy == redif_scale_policy::mean_local_distance && count > 0) {
			scales[i] = total / static_cast<double>(count);
		} else if (policy == redif_scale_policy::median_local_distance) {
			scales[i] = positive_or_default(std::move(positives), global_scale);
		} else if (policy == redif_scale_policy::kth_local_distance && !positives.empty()) {
			scales[i] = *std::max_element(positives.begin(), positives.end());
		}
	}
	return scales;
}

inline auto redif_refusal_diagnostics(std::size_t record_count, std::size_t neighbors,
									  std::string representation, std::string reason)
	-> RedifLocalRelationDiagnostics
{
	RedifLocalRelationDiagnostics diagnostics;
	diagnostics.node_count = record_count;
	diagnostics.neighbors = std::min(neighbors, record_count > 0 ? record_count - 1 : 0);
	diagnostics.representation = std::move(representation);
	diagnostics.exactness = "refused";
	diagnostics.exact = false;
	diagnostics.refused = true;
	diagnostics.refusal_reason = std::move(reason);
	return diagnostics;
}

inline auto summarize_redif_sparse_operator(const RedifSparseOperator &op) -> RedifOperatorDiagnostics
{
	RedifOperatorDiagnostics diagnostics;
	diagnostics.node_count = op.node_count;
	diagnostics.neighbors = op.neighbors;
	diagnostics.local_relation_representation = op.local_relation_diagnostics.representation;
	diagnostics.local_relation_exactness = op.local_relation_diagnostics.exactness;
	diagnostics.local_relation_directed_entries = op.local_relation_diagnostics.symmetric_entries;
	diagnostics.local_relation_distance_evaluations = op.local_relation_diagnostics.distance_evaluations;
	diagnostics.local_relation_candidate_count = op.local_relation_diagnostics.candidate_count;
	diagnostics.local_relation_candidate_universe = op.local_relation_diagnostics.candidate_universe;
	diagnostics.local_relation_chunk_size = op.local_relation_diagnostics.chunk_size;
	diagnostics.local_relation_chunk_count = op.local_relation_diagnostics.chunk_count;
	diagnostics.local_relation_candidate_fraction = op.local_relation_diagnostics.candidate_fraction;
	diagnostics.local_relation_exact = op.local_relation_diagnostics.exact;
	diagnostics.local_relation_refused = op.local_relation_diagnostics.refused;
	diagnostics.local_relation_refusal_reason = op.local_relation_diagnostics.refusal_reason;
	diagnostics.reversible = true;
	if (op.node_count == 0) {
		return diagnostics;
	}

	diagnostics.minimum_degree = std::numeric_limits<double>::infinity();
	diagnostics.maximum_degree = 0.0;
	diagnostics.minimum_transition_row_sum = std::numeric_limits<double>::infinity();
	diagnostics.maximum_transition_row_sum = 0.0;
	diagnostics.minimum_transition_escape_probability = std::numeric_limits<double>::infinity();
	for (std::size_t i = 0; i < op.node_count; ++i) {
		diagnostics.minimum_degree = std::min(diagnostics.minimum_degree, op.degree[i]);
		diagnostics.maximum_degree = std::max(diagnostics.maximum_degree, op.degree[i]);
		diagnostics.minimum_local_scale =
			i == 0 ? op.local_scale[i] : std::min(diagnostics.minimum_local_scale, op.local_scale[i]);
		diagnostics.maximum_local_scale = std::max(diagnostics.maximum_local_scale, op.local_scale[i]);
		const auto row_sum = op.transition_row_sum(i);
		diagnostics.minimum_transition_row_sum = std::min(diagnostics.minimum_transition_row_sum, row_sum);
		diagnostics.maximum_transition_row_sum = std::max(diagnostics.maximum_transition_row_sum, row_sum);
		double self_transition = 0.0;
		for (const auto &entry : op.rows[i]) {
			diagnostics.maximum_affinity = std::max(diagnostics.maximum_affinity, entry.affinity);
			if (entry.affinity > 0.0) {
				diagnostics.minimum_positive_affinity =
					diagnostics.minimum_positive_affinity == 0.0
						? entry.affinity
						: std::min(diagnostics.minimum_positive_affinity, entry.affinity);
			}
			if (entry.index == i) {
				self_transition = entry.transition;
			}
		}
		const auto escape_probability = std::max(0.0, row_sum - self_transition);
		diagnostics.maximum_self_transition_probability =
			std::max(diagnostics.maximum_self_transition_probability, self_transition);
		diagnostics.minimum_transition_escape_probability =
			std::min(diagnostics.minimum_transition_escape_probability, escape_probability);
	}
	diagnostics.degree_ratio =
		diagnostics.maximum_degree > 0.0 ? diagnostics.minimum_degree / diagnostics.maximum_degree : 0.0;
	if (!std::isfinite(diagnostics.minimum_transition_escape_probability)) {
		diagnostics.minimum_transition_escape_probability = 0.0;
	}
	diagnostics.spectral_gap_proxy_value = diagnostics.minimum_transition_escape_probability;

	std::vector<unsigned char> visited(op.node_count, 0);
	for (std::size_t seed = 0; seed < op.node_count; ++seed) {
		if (visited[seed]) {
			continue;
		}
		++diagnostics.component_count;
		std::vector<std::size_t> stack{seed};
		visited[seed] = 1;
		while (!stack.empty()) {
			const auto node = stack.back();
			stack.pop_back();
			for (const auto &entry : op.rows[node]) {
				if (!visited[entry.index] && entry.affinity > 0.0) {
					visited[entry.index] = 1;
					stack.push_back(entry.index);
				}
			}
		}
	}
	return diagnostics;
}

inline auto redif_sparse_operator_from_exact_rows(
	std::size_t record_count, std::vector<std::vector<std::pair<std::size_t, double>>> rows,
	RedifLocalRelationDiagnostics diagnostics, redif_options options) -> RedifSparseOperator
{
	const std::size_t k = std::min(options.neighbors, record_count > 0 ? record_count - 1 : 0);
	RedifSparseOperator op;
	op.node_count = record_count;
	op.neighbors = k;
	op.local_scale_policy = redif_scale_policy_name(options.scale_policy);
	op.local_relation_diagnostics = std::move(diagnostics);
	op.local_scale = redif_sparse_local_scales(rows, options.scale_policy);
	op.rows.assign(record_count, {});
	op.degree.assign(record_count, 0.0);

	double degree_total = 0.0;
	for (std::size_t i = 0; i < record_count; ++i) {
		op.rows[i].reserve(rows[i].size());
		double weighted_degree = 0.0;
		for (const auto &entry : rows[i]) {
			const auto affinity = redif_heat_affinity(true, entry.second, op.local_scale[i],
													 op.local_scale[entry.first]);
			op.rows[i].push_back(RedifSparseEntry{entry.first, entry.second, affinity, 0.0});
			weighted_degree += affinity;
		}
		if (weighted_degree == 0.0) {
			op.rows[i].push_back(RedifSparseEntry{i, 0.0, 1.0, 1.0});
			weighted_degree = 1.0;
		}
		op.degree[i] = weighted_degree;
		degree_total += weighted_degree;
		for (auto &entry : op.rows[i]) {
			entry.transition = entry.affinity / weighted_degree;
		}
	}

	op.stationary.assign(record_count, 0.0);
	if (degree_total > 0.0) {
		for (std::size_t i = 0; i < record_count; ++i) {
			op.stationary[i] = op.degree[i] / degree_total;
		}
	} else if (record_count > 0) {
		for (std::size_t i = 0; i < record_count; ++i) {
			op.stationary[i] = 1.0 / static_cast<double>(record_count);
		}
	}
	op.diagnostics = summarize_redif_sparse_operator(op);
	return op;
}

inline auto require_redif_sparse_relation_budget(std::size_t record_count, std::size_t neighbors,
												 const redif_options &options) -> void
{
	const auto k = std::min(neighbors, record_count > 0 ? record_count - 1 : 0);
	const auto directed_entries = checked_dense_product(record_count, k, "Redif exact local relation");
	const auto symmetric_entries = checked_dense_product(directed_entries, std::size_t{2},
														 "Redif exact local relation");
	const auto estimated_memory = checked_dense_product(symmetric_entries, sizeof(RedifSparseEntry),
														"Redif exact local relation");
	if (options.max_distance_evaluations > 0 && directed_entries > options.max_distance_evaluations) {
		throw MetricInputError(
			"Redif exact local relation exceeds max_distance_evaluations: records=" +
			std::to_string(record_count) + " neighbors=" + std::to_string(k) +
			" estimated_distance_evaluations=" + std::to_string(directed_entries) +
			" distance_evaluation_budget=" + std::to_string(options.max_distance_evaluations));
	}
	if (options.max_memory_bytes > 0 && estimated_memory > options.max_memory_bytes) {
		throw MetricInputError("Redif exact local relation exceeds max_memory_bytes: records=" +
							   std::to_string(record_count) + " neighbors=" + std::to_string(k) +
							   " estimated_bytes=" + std::to_string(estimated_memory) +
							   " budget_bytes=" + std::to_string(options.max_memory_bytes));
	}
}

inline auto summarize_redif_operator(const RedifOperator &op) -> RedifOperatorDiagnostics
{
	RedifOperatorDiagnostics diagnostics;
	diagnostics.node_count = op.node_count;
	diagnostics.neighbors = op.neighbors;
	diagnostics.local_relation_representation = op.local_relation_diagnostics.representation;
	diagnostics.local_relation_exactness = op.local_relation_diagnostics.exactness;
	diagnostics.local_relation_directed_entries =
		op.local_relation_diagnostics.symmetric_entries > 0
			? op.local_relation_diagnostics.symmetric_entries
			: redif_directed_relation_entry_count(op.local_relation);
	diagnostics.local_relation_distance_evaluations = op.local_relation_diagnostics.distance_evaluations;
	diagnostics.dense_distance_evaluations =
		op.local_relation_diagnostics.representation == "dense_distance_matrix"
			? dense_unordered_pair_count(op.node_count)
			: 0;
	diagnostics.local_relation_candidate_count = op.local_relation_diagnostics.candidate_count;
	diagnostics.local_relation_candidate_universe = op.local_relation_diagnostics.candidate_universe;
	diagnostics.local_relation_chunk_size = op.local_relation_diagnostics.chunk_size;
	diagnostics.local_relation_chunk_count = op.local_relation_diagnostics.chunk_count;
	diagnostics.local_relation_candidate_fraction = op.local_relation_diagnostics.candidate_fraction;
	diagnostics.local_relation_exact = op.local_relation_diagnostics.exact;
	diagnostics.local_relation_refused = op.local_relation_diagnostics.refused;
	diagnostics.local_relation_refusal_reason = op.local_relation_diagnostics.refusal_reason;
	diagnostics.reversible = true;
	if (op.node_count == 0) {
		return diagnostics;
	}

	diagnostics.minimum_degree = std::numeric_limits<double>::infinity();
	diagnostics.maximum_degree = 0.0;
	diagnostics.minimum_transition_row_sum = std::numeric_limits<double>::infinity();
	diagnostics.maximum_transition_row_sum = 0.0;
	diagnostics.minimum_transition_escape_probability = std::numeric_limits<double>::infinity();
	for (std::size_t i = 0; i < op.node_count; ++i) {
		diagnostics.minimum_degree = std::min(diagnostics.minimum_degree, op.degree[i]);
		diagnostics.maximum_degree = std::max(diagnostics.maximum_degree, op.degree[i]);
		diagnostics.minimum_local_scale =
			i == 0 ? op.local_scale[i] : std::min(diagnostics.minimum_local_scale, op.local_scale[i]);
		diagnostics.maximum_local_scale = std::max(diagnostics.maximum_local_scale, op.local_scale[i]);
		const auto row_sum = op.transition_row_sum(i);
		diagnostics.minimum_transition_row_sum = std::min(diagnostics.minimum_transition_row_sum, row_sum);
		diagnostics.maximum_transition_row_sum = std::max(diagnostics.maximum_transition_row_sum, row_sum);
		const auto self_transition = op.transition[i][i];
		const auto escape_probability = std::max(0.0, row_sum - self_transition);
		diagnostics.maximum_self_transition_probability =
			std::max(diagnostics.maximum_self_transition_probability, self_transition);
		diagnostics.minimum_transition_escape_probability =
			std::min(diagnostics.minimum_transition_escape_probability, escape_probability);
		for (std::size_t j = 0; j < op.node_count; ++j) {
			diagnostics.maximum_affinity = std::max(diagnostics.maximum_affinity, op.affinity[i][j]);
			if (op.affinity[i][j] > 0.0) {
				diagnostics.minimum_positive_affinity =
					diagnostics.minimum_positive_affinity == 0.0
						? op.affinity[i][j]
						: std::min(diagnostics.minimum_positive_affinity, op.affinity[i][j]);
			}
		}
		for (std::size_t j = i + 1; j < op.node_count; ++j) {
			if (std::abs(op.affinity[i][j] - op.affinity[j][i]) > 1e-12) {
				diagnostics.reversible = false;
			}
		}
	}
	diagnostics.degree_ratio =
		diagnostics.maximum_degree > 0.0 ? diagnostics.minimum_degree / diagnostics.maximum_degree : 0.0;
	if (!std::isfinite(diagnostics.minimum_transition_escape_probability)) {
		diagnostics.minimum_transition_escape_probability = 0.0;
	}
	diagnostics.spectral_gap_proxy_value = diagnostics.minimum_transition_escape_probability;
	std::vector<unsigned char> visited(op.node_count, 0);
	for (std::size_t seed = 0; seed < op.node_count; ++seed) {
		if (visited[seed]) {
			continue;
		}
		++diagnostics.component_count;
		std::vector<std::size_t> stack{seed};
		visited[seed] = 1;
		while (!stack.empty()) {
			const auto node = stack.back();
			stack.pop_back();
			for (std::size_t next = 0; next < op.node_count; ++next) {
				if (!visited[next] && op.affinity[node][next] > 0.0) {
					visited[next] = 1;
					stack.push_back(next);
				}
			}
		}
	}
	return diagnostics;
}

inline auto redif_operator_from_local_relation(RedifLocalRelation local, std::size_t neighbors,
											   redif_scale_policy scale_policy) -> RedifOperator
{
	const std::size_t n = local.distances.size();
	RedifOperator op;
	op.node_count = n;
	op.neighbors = neighbors;
	op.local_scale_policy = redif_scale_policy_name(scale_policy);
	op.local_relation = std::move(local.connected);
	op.local_distances = std::move(local.distances);
	op.local_relation_diagnostics.node_count = n;
	op.local_relation_diagnostics.neighbors = neighbors;
	op.local_relation_diagnostics.representation = "dense_distance_matrix";
	op.local_relation_diagnostics.exactness = "exact";
	op.local_relation_diagnostics.symmetric_entries = redif_directed_relation_entry_count(op.local_relation);
	op.local_relation_diagnostics.distance_evaluations = dense_unordered_pair_count(n);
	op.local_relation_diagnostics.exact = true;
	op.local_scale = redif_local_scales(RedifLocalRelation{op.local_distances, op.local_relation}, scale_policy);
	op.affinity.assign(n, std::vector<double>(n, 0.0));
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			op.affinity[i][j] = redif_heat_affinity(op.local_relation[i][j] != 0, op.local_distances[i][j],
													op.local_scale[i], op.local_scale[j]);
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
			op.affinity[i][i] = 1.0;
			weighted_degree = 1.0;
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

	op.sparse_rows.assign(n, {});
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			if (op.affinity[i][j] > 0.0) {
				op.sparse_rows[i].push_back(
					RedifSparseEntry{j, op.local_distances[i][j], op.affinity[i][j], op.transition[i][j]});
			}
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

inline auto redif_operator_from_local_distances(std::vector<std::vector<double>> local_distances,
												std::size_t neighbors) -> RedifOperator
{
	RedifLocalRelation local;
	const std::size_t n = local_distances.size();
	local.connected.assign(n, std::vector<unsigned char>(n, 0));
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			local.connected[i][j] = i != j && local_distances[i][j] > 0.0 ? 1 : 0;
		}
	}
	local.distances = std::move(local_distances);
	return redif_operator_from_local_relation(std::move(local), neighbors, redif_scale_policy::mean_local_distance);
}

inline auto redif_operator_from_distances(const std::vector<std::vector<double>> &distances, std::size_t neighbors,
										  redif_scale_policy scale_policy = redif_scale_policy::mean_local_distance)
	-> RedifOperator
{
	auto local = local_knn_relation(distances, neighbors);
	return redif_operator_from_local_relation(
		std::move(local), std::min(neighbors, distances.size() > 0 ? distances.size() - 1 : 0), scale_policy);
}

inline auto redif_operator_from_sampled_distances(const std::vector<std::vector<double>> &distances,
												  std::size_t neighbors,
												  redif_scale_policy scale_policy,
												  redif_sampled_relation_options sample_options) -> RedifOperator
{
	RedifLocalRelationDiagnostics diagnostics;
	auto local = sampled_chunked_local_relation(distances, neighbors, sample_options, &diagnostics);
	auto op = redif_operator_from_local_relation(
		std::move(local), std::min(neighbors, distances.size() > 0 ? distances.size() - 1 : 0), scale_policy);
	op.local_relation_diagnostics = std::move(diagnostics);
	op.diagnostics = summarize_redif_operator(op);
	return op;
}

inline auto finalize_stability(RedifStabilityDiagnostics &diagnostics, double tolerance,
							   double marginal_tolerance) -> void
{
	if (diagnostics.maximum_pivot_abs > 0.0) {
		diagnostics.pivot_ratio = diagnostics.minimum_pivot_abs / diagnostics.maximum_pivot_abs;
		diagnostics.singularity_margin = diagnostics.minimum_pivot_abs;
	}
	if (diagnostics.singularity_margin <= tolerance) {
		diagnostics.status = "singular";
		diagnostics.stable = false;
		diagnostics.marginal = false;
	} else if (diagnostics.singularity_margin <= marginal_tolerance) {
		diagnostics.status = "marginal";
		diagnostics.stable = true;
		diagnostics.marginal = true;
	} else {
		diagnostics.status = "stable";
		diagnostics.stable = true;
		diagnostics.marginal = false;
	}
}

inline auto observe_pivot(RedifStabilityDiagnostics &diagnostics, double pivot_abs) -> void
{
	if (diagnostics.maximum_pivot_abs == 0.0) {
		diagnostics.minimum_pivot_abs = pivot_abs;
		diagnostics.maximum_pivot_abs = pivot_abs;
	} else {
		diagnostics.minimum_pivot_abs = std::min(diagnostics.minimum_pivot_abs, pivot_abs);
		diagnostics.maximum_pivot_abs = std::max(diagnostics.maximum_pivot_abs, pivot_abs);
	}
}

inline auto combine_stability(RedifStabilityDiagnostics &target, const RedifStabilityDiagnostics &source,
							  double tolerance, double marginal_tolerance) -> void
{
	if (source.maximum_pivot_abs == 0.0) {
		return;
	}
	if (target.maximum_pivot_abs == 0.0) {
		target.minimum_pivot_abs = source.minimum_pivot_abs;
		target.maximum_pivot_abs = source.maximum_pivot_abs;
	} else {
		target.minimum_pivot_abs = std::min(target.minimum_pivot_abs, source.minimum_pivot_abs);
		target.maximum_pivot_abs = std::max(target.maximum_pivot_abs, source.maximum_pivot_abs);
	}
	finalize_stability(target, tolerance, marginal_tolerance);
}

inline auto stable_step_diagnostics(std::string status = "stable_no_linear_solve") -> RedifStabilityDiagnostics
{
	RedifStabilityDiagnostics diagnostics;
	diagnostics.status = std::move(status);
	diagnostics.minimum_pivot_abs = 1.0;
	diagnostics.maximum_pivot_abs = 1.0;
	diagnostics.pivot_ratio = 1.0;
	diagnostics.singularity_margin = 1.0;
	diagnostics.stable = true;
	diagnostics.marginal = false;
	return diagnostics;
}

inline auto solve_linear_system(std::vector<std::vector<double>> matrix, std::vector<double> rhs,
								const std::string &system_name = "Redif linear system",
								RedifStabilityDiagnostics *stability = nullptr,
								double tolerance = 1.0e-12,
								double marginal_tolerance = 1.0e-8)
	-> std::vector<double>
{
	const std::size_t n = matrix.size();
	RedifStabilityDiagnostics local_stability;
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
		observe_pivot(local_stability, best_abs);
		if (best_abs <= tolerance) {
			finalize_stability(local_stability, tolerance, marginal_tolerance);
			if (stability != nullptr) {
				*stability = local_stability;
			}
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
	finalize_stability(local_stability, tolerance, marginal_tolerance);
	if (stability != nullptr) {
		*stability = local_stability;
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

inline auto summarize_state_entropy(std::size_t step, const RedifOperator &op,
									const RedifStabilityDiagnostics &stability,
									const std::vector<std::vector<double>> &state) -> RedifStepDiagnostics
{
	RedifStepDiagnostics diagnostics;
	diagnostics.step = step;
	diagnostics.operator_diagnostics = op.diagnostics;
	diagnostics.stability = stability;
	diagnostics.stationary = op.stationary;
	diagnostics.minimum_shannon_entropy = std::numeric_limits<double>::infinity();
	diagnostics.maximum_shannon_entropy = 0.0;
	diagnostics.maximum_relative_entropy_to_current_stationary = 0.0;
	for (const auto &measure : state) {
		const auto entropy = shannon_entropy(measure);
		diagnostics.minimum_shannon_entropy = std::min(diagnostics.minimum_shannon_entropy, entropy);
		diagnostics.maximum_shannon_entropy = std::max(diagnostics.maximum_shannon_entropy, entropy);
		const auto relative = relative_entropy(measure, op.stationary);
		if (std::isfinite(relative)) {
			diagnostics.maximum_relative_entropy_to_current_stationary =
				std::max(diagnostics.maximum_relative_entropy_to_current_stationary, relative);
		} else {
			diagnostics.maximum_relative_entropy_to_current_stationary = relative;
		}
	}
	if (state.empty()) {
		diagnostics.minimum_shannon_entropy = 0.0;
	}
	return diagnostics;
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
									  double euler_step,
									  RedifStabilityDiagnostics *stability = nullptr,
									  double stability_tolerance = 1.0e-12,
									  double marginal_stability_tolerance = 1.0e-8) -> std::vector<std::vector<double>>
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
	RedifStabilityDiagnostics aggregate_stability;
	for (std::size_t coordinate = 0; coordinate < n; ++coordinate) {
		std::vector<double> rhs(n, 0.0);
		for (std::size_t row = 0; row < n; ++row) {
			rhs[row] = op.degree[row] * state[row][coordinate];
		}
		RedifStabilityDiagnostics coordinate_stability;
		std::vector<double> solution;
		try {
			solution = solve_linear_system(left, std::move(rhs), "Redif implicit Euler system",
										   &coordinate_stability, stability_tolerance,
										   marginal_stability_tolerance);
		} catch (...) {
			combine_stability(aggregate_stability, coordinate_stability, stability_tolerance,
							  marginal_stability_tolerance);
			if (stability != nullptr) {
				*stability = aggregate_stability.maximum_pivot_abs == 0.0 ? coordinate_stability
																		: aggregate_stability;
			}
			throw;
		}
		combine_stability(aggregate_stability, coordinate_stability, stability_tolerance,
						  marginal_stability_tolerance);
		for (std::size_t row = 0; row < n; ++row) {
			next[row][coordinate] = solution[row];
		}
	}

	for (auto &row : next) {
		normalize_probability_row(row);
	}
	if (stability != nullptr) {
		*stability = aggregate_stability.maximum_pivot_abs == 0.0 ? stable_step_diagnostics() : aggregate_stability;
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
											 double euler_step,
											 RedifStabilityDiagnostics *stability = nullptr,
											 double stability_tolerance = 1.0e-12,
											 double marginal_stability_tolerance = 1.0e-8) -> std::vector<std::vector<double>>
{
	validate_redif_state(op, state);
	validate_redif_step(euler_step);
	const std::size_t n = op.node_count;
	const auto left = transpose(forward_euler_matrix(op, euler_step));
	std::vector<std::vector<double>> previous(n, std::vector<double>(n, 0.0));
	RedifStabilityDiagnostics aggregate_stability;
	for (std::size_t row = 0; row < n; ++row) {
		RedifStabilityDiagnostics row_stability;
		std::vector<double> solution;
		try {
			solution = solve_linear_system(left, state[row], "Redif inverse forward Euler system",
										   &row_stability, stability_tolerance,
										   marginal_stability_tolerance);
		} catch (...) {
			combine_stability(aggregate_stability, row_stability, stability_tolerance,
							  marginal_stability_tolerance);
			if (stability != nullptr) {
				*stability = aggregate_stability.maximum_pivot_abs == 0.0 ? row_stability
																		: aggregate_stability;
			}
			throw;
		}
		combine_stability(aggregate_stability, row_stability, stability_tolerance,
						  marginal_stability_tolerance);
		normalize_probability_row(solution);
		previous[row] = std::move(solution);
	}
	if (stability != nullptr) {
		*stability = aggregate_stability.maximum_pivot_abs == 0.0 ? stable_step_diagnostics() : aggregate_stability;
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

inline auto make_redif_transport_diagnostics(std::size_t record_count, const redif_options &options)
	-> RedifTransportDiagnostics
{
	RedifTransportDiagnostics diagnostics;
	diagnostics.measure_count = record_count;
	diagnostics.pair_count = record_count < 2 ? 0 : (record_count * (record_count - 1)) / 2;
	diagnostics.transport_problem_count = redif_transport_problem_count(record_count, options.iterations);
	diagnostics.max_transport_problems = options.max_transport_problems;
	diagnostics.support_atom_count = record_count;
	diagnostics.max_transport_support_atoms = options.max_transport_support_atoms;
	diagnostics.support_mass_floor = options.transport_support_mass_floor;
	diagnostics.exact = true;
	diagnostics.exactness = "exact";
	return diagnostics;
}

inline auto redif_transport_measure(const std::vector<double> &measure, const redif_options &options,
									RedifTransportDiagnostics *diagnostics) -> std::vector<double>
{
	const std::size_t n = measure.size();
	const bool bounded_support =
		options.max_transport_support_atoms > 0 && options.max_transport_support_atoms < n;
	const bool mass_floor = options.transport_support_mass_floor > 0.0;
	if (!options.allow_transport_support_truncation || (!bounded_support && !mass_floor)) {
		return measure;
	}

	std::vector<unsigned char> keep(n, 1);
	if (mass_floor) {
		for (std::size_t index = 0; index < n; ++index) {
			keep[index] = measure[index] > options.transport_support_mass_floor ? 1 : 0;
		}
	}
	if (bounded_support) {
		std::vector<std::size_t> order(n);
		for (std::size_t index = 0; index < n; ++index) {
			order[index] = index;
		}
		std::sort(order.begin(), order.end(), [&](std::size_t lhs, std::size_t rhs) {
			if (measure[lhs] != measure[rhs]) {
				return measure[lhs] > measure[rhs];
			}
			return lhs < rhs;
		});
		std::vector<unsigned char> bounded_keep(n, 0);
		std::size_t retained = 0;
		for (const auto index : order) {
			if (!keep[index]) {
				continue;
			}
			if (retained >= options.max_transport_support_atoms) {
				break;
			}
			bounded_keep[index] = 1;
			++retained;
		}
		keep = std::move(bounded_keep);
	}

	bool any_retained = false;
	for (const auto value : keep) {
		any_retained = any_retained || value != 0;
	}
	if (!any_retained && n > 0) {
		auto top = std::size_t{0};
		for (std::size_t index = 1; index < n; ++index) {
			if (measure[index] > measure[top]) {
				top = index;
			}
		}
		keep[top] = 1;
	}

	std::vector<double> truncated(n, 0.0);
	double retained_mass = 0.0;
	double discarded_mass = 0.0;
	bool removed_atom = false;
	for (std::size_t index = 0; index < n; ++index) {
		if (keep[index]) {
			truncated[index] = measure[index];
			retained_mass += measure[index];
		} else {
			discarded_mass += measure[index];
			removed_atom = true;
		}
	}
	if (!(retained_mass > 0.0)) {
		throw std::runtime_error("Redif transport support truncation removed all measure mass");
	}
	for (auto &value : truncated) {
		value /= retained_mass;
	}
	if (diagnostics != nullptr && removed_atom) {
		diagnostics->support_truncated = true;
		++diagnostics->truncated_measure_count;
		diagnostics->discarded_mass_total += discarded_mass;
		diagnostics->maximum_discarded_mass =
			std::max(diagnostics->maximum_discarded_mass, discarded_mass);
		if (discarded_mass > 1e-15) {
			diagnostics->exact = false;
			diagnostics->exactness = "non_exact_support_truncated_transport";
		}
	}
	return truncated;
}

inline auto transition_step_transport(const std::vector<std::vector<double>> &state,
									  const std::vector<std::vector<double>> &next,
									  const Wasserstein<double> &transport,
									  const redif_options &options,
									  RedifTransportDiagnostics *diagnostics)
	-> std::vector<double>
{
	std::vector<double> step(state.size(), 0.0);
	for (std::size_t record = 0; record < state.size(); ++record) {
		const auto left = redif_transport_measure(state[record], options, diagnostics);
		const auto right = redif_transport_measure(next[record], options, diagnostics);
		step[record] = transport(left, right);
	}
	return step;
}

inline auto redif_step_stability_weight(const RedifStabilityDiagnostics &stability,
										const redif_options &options) -> double
{
	if (!stability.stable || stability.status == "singular") {
		return 0.0;
	}
	if (!stability.marginal) {
		return 1.0;
	}
	if (!(options.marginal_stability_tolerance > 0.0)) {
		return 0.0;
	}
	const auto weight = stability.singularity_margin / options.marginal_stability_tolerance;
	return std::max(0.0, std::min(1.0, weight));
}

inline auto redif_result_stability_weight(const std::vector<RedifStepDiagnostics> &steps,
										  const redif_options &options) -> double
{
	if (steps.empty()) {
		return 1.0;
	}
	double weight = 1.0;
	for (const auto &step : steps) {
		weight = std::min(weight, redif_step_stability_weight(step.stability, options));
	}
	return weight;
}

inline auto make_measure_result(const std::vector<RecordId> &ids,
								const std::vector<std::vector<std::vector<double>>> &frames,
								const std::vector<std::vector<double>> &step_transport,
								std::vector<double> initial_stationary,
								std::vector<double> terminal_stationary,
								std::vector<RedifOperatorDiagnostics> operator_diagnostics,
								std::vector<RedifStepDiagnostics> step_diagnostics,
								RedifTransportDiagnostics transport_diagnostics,
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
				relative_entropy(path.terminal_measure(), initial_stationary),
				terminal_stationary.empty() ? 0.0 : relative_entropy(path.terminal_measure(), terminal_stationary)});
		}
	}

	bool exact_result = transport_diagnostics.exact;
	for (const auto &diagnostics : operator_diagnostics) {
		exact_result = exact_result && diagnostics.local_relation_exact && !diagnostics.local_relation_refused;
	}

	return RedifMeasureResult{std::move(paths),
							  n,
							  neighbors,
							  options.iterations,
							  options.euler_step,
							  options.adaptive_geometry,
							  exact_result,
							  std::move(operator_name),
							  std::move(strategy),
							  std::move(representation),
							  std::move(initial_stationary),
							  std::move(terminal_stationary),
							  std::move(entropy_diagnostics),
							  std::move(operator_diagnostics),
							  std::move(step_diagnostics),
							  std::move(transport_diagnostics)};
}

} // namespace detail

inline auto RedifMeasureResult::summaries() const -> std::vector<RedifMeasureSummary>
{
	std::vector<RedifMeasureSummary> result;
	result.reserve(paths.size());
	for (const auto &path : paths) {
		const auto &terminal = path.terminal_measure();
		std::size_t top_index = 0;
		double top_mass = terminal.empty() ? 0.0 : terminal.front();
		for (std::size_t index = 1; index < terminal.size(); ++index) {
			if (terminal[index] > top_mass) {
				top_mass = terminal[index];
				top_index = index;
			}
		}
		const auto initial_entropy = path.measures.empty() ? 0.0 : detail::shannon_entropy(path.measures.front());
		const auto terminal_entropy = detail::shannon_entropy(terminal);
		result.push_back(RedifMeasureSummary{path.id,
											 path.transport_path_length,
											 initial_entropy,
											 terminal_entropy,
											 terminal_entropy - initial_entropy,
											 top_index,
											 top_mass});
	}
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto redif_operator(const Space &space, redif_options options = {}) -> RedifOperator
{
	detail::validate_redif_options(space.size(), options);
	const std::size_t k = std::min(options.neighbors, space.size() > 0 ? space.size() - 1 : 0);
	return detail::redif_operator_from_distances(detail::distance_matrix(space), k, options.scale_policy);
}

inline auto redif_operator_from_distance_matrix(const std::vector<std::vector<double>> &distances,
												redif_options options = {}) -> RedifOperator
{
	detail::validate_redif_options(distances.size(), options);
	const std::size_t k = std::min(options.neighbors, distances.size() > 0 ? distances.size() - 1 : 0);
	return detail::redif_operator_from_distances(distances, k, options.scale_policy);
}

inline auto redif_operator_from_sampled_distance_matrix(
	const std::vector<std::vector<double>> &distances, redif_options options = {},
	redif_sampled_relation_options sample_options = {}) -> RedifOperator
{
	detail::validate_redif_options(distances.size(), options);
	const std::size_t k = std::min(options.neighbors, distances.size() > 0 ? distances.size() - 1 : 0);
	return detail::redif_operator_from_sampled_distances(distances, k, options.scale_policy, sample_options);
}

inline auto redif_operator_from_sparse_operator(const RedifSparseOperator &sparse) -> RedifOperator
{
	const std::size_t n = sparse.node_count;
	RedifOperator op;
	op.node_count = n;
	op.neighbors = sparse.neighbors;
	op.affinity_kernel = sparse.affinity_kernel;
	op.local_scale_policy = sparse.local_scale_policy;
	op.generator = sparse.generator;
	op.local_distances.assign(n, std::vector<double>(n, 0.0));
	op.local_relation.assign(n, std::vector<unsigned char>(n, 0));
	op.affinity.assign(n, std::vector<double>(n, 0.0));
	op.transition.assign(n, std::vector<double>(n, 0.0));
	op.laplacian.assign(n, std::vector<double>(n, 0.0));
	op.local_scale = sparse.local_scale;
	op.degree = sparse.degree;
	op.stationary = sparse.stationary;
	op.sparse_rows = sparse.rows;
	op.local_relation_diagnostics = sparse.local_relation_diagnostics;

	for (std::size_t i = 0; i < n; ++i) {
		op.laplacian[i][i] = op.degree[i];
		for (const auto &entry : sparse.rows[i]) {
			op.affinity[i][entry.index] = entry.affinity;
			op.transition[i][entry.index] = entry.transition;
			op.laplacian[i][entry.index] -= entry.affinity;
			if (entry.index != i) {
				op.local_distances[i][entry.index] = entry.distance;
				op.local_relation[i][entry.index] = 1;
			}
		}
	}
	op.diagnostics = detail::summarize_redif_operator(op);
	return op;
}

inline auto redif_sparse_operator_from_exact_local_relation(
	std::size_t record_count, const std::vector<RedifLocalRelationEntry> &entries,
	redif_options options = {}) -> RedifSparseOperator
{
	detail::validate_redif_options(record_count, options, false);
	const std::size_t k = std::min(options.neighbors, record_count > 0 ? record_count - 1 : 0);
	detail::require_redif_sparse_relation_budget(record_count, k, options);
	RedifLocalRelationDiagnostics diagnostics;
	auto rows = detail::sparse_relation_from_exact_entries(record_count, k, entries, &diagnostics);
	return detail::redif_sparse_operator_from_exact_rows(record_count, std::move(rows), std::move(diagnostics),
														 options);
}

inline auto redif_local_relation_refusal(std::size_t record_count, std::size_t neighbors,
										 std::string representation, std::string reason)
	-> RedifLocalRelationDiagnostics
{
	return detail::redif_refusal_diagnostics(record_count, neighbors, std::move(representation), std::move(reason));
}

template <typename NeighborProvider>
auto redif_sparse_operator_from_exact_neighbor_provider(std::size_t record_count, NeighborProvider provider,
														redif_options options = {}) -> RedifSparseOperator
{
	detail::validate_redif_options(record_count, options, false);
	const std::size_t k = std::min(options.neighbors, record_count > 0 ? record_count - 1 : 0);
	detail::require_redif_sparse_relation_budget(record_count, k, options);
	std::vector<RedifLocalRelationEntry> entries;
	entries.reserve(record_count * k);
	for (std::size_t source = 0; source < record_count; ++source) {
		const auto row = provider(source, k);
		for (const auto &entry : row) {
			entries.push_back(RedifLocalRelationEntry{source, entry.first, static_cast<double>(entry.second)});
		}
	}
	return redif_sparse_operator_from_exact_local_relation(record_count, entries, options);
}

template <typename NeighborProvider>
auto redif_operator_from_exact_neighbor_provider(std::size_t record_count, NeighborProvider provider,
												 redif_options options = {}) -> RedifOperator
{
	return redif_operator_from_sparse_operator(
		redif_sparse_operator_from_exact_neighbor_provider(record_count, std::move(provider), options));
}

inline auto redif_forward_noise_step(const RedifOperator &op, const std::vector<std::vector<double>> &state,
									 double euler_step) -> std::vector<std::vector<double>>
{
	return detail::redif_forward_euler_step(op, state, euler_step);
}

inline auto redif_inverse_noise_step(const RedifOperator &op, const std::vector<std::vector<double>> &state,
									 double euler_step,
									 RedifStabilityDiagnostics *stability = nullptr,
									 double stability_tolerance = 1.0e-12,
									 double marginal_stability_tolerance = 1.0e-8) -> std::vector<std::vector<double>>
{
	return detail::redif_inverse_forward_euler_step(op, state, euler_step, stability,
													stability_tolerance, marginal_stability_tolerance);
}

inline auto redif_record_ids_from_count(std::size_t count) -> std::vector<RecordId>
{
	std::vector<RecordId> ids;
	ids.reserve(count);
	for (std::size_t index = 0; index < count; ++index) {
		ids.push_back(RecordId::from_index(index));
	}
	return ids;
}

inline auto redif_validate_record_ids(const std::vector<RecordId> &ids, std::size_t count) -> void
{
	if (ids.size() != count) {
		throw std::invalid_argument("Redif record id count must match the distance matrix");
	}
}

inline auto redif_remove_noise_from_distance_matrix(const std::vector<std::vector<double>> &source_distances,
													redif_options options = {},
													std::vector<RecordId> ids = {}) -> RedifMeasureResult
{
	detail::validate_redif_options(source_distances.size(), options);
	detail::validate_redif_distance_matrix(source_distances);
	const std::size_t n = source_distances.size();
	const std::size_t k = std::min(options.neighbors, n > 0 ? n - 1 : 0);
	if (ids.empty()) {
		ids = redif_record_ids_from_count(n);
	}
	redif_validate_record_ids(ids, n);

	Wasserstein<double> transport(source_distances);
	auto transport_diagnostics = detail::make_redif_transport_diagnostics(n, options);
	std::vector<std::vector<std::vector<double>>> frames;
	std::vector<std::vector<double>> step_transport;
	std::vector<RedifOperatorDiagnostics> operator_diagnostics;
	std::vector<RedifStepDiagnostics> step_diagnostics;
	std::vector<double> initial_stationary;
	std::vector<double> terminal_stationary;
	frames.reserve(options.iterations + 1);
	step_transport.reserve(options.iterations);
	operator_diagnostics.reserve(options.iterations);
	step_diagnostics.reserve(options.iterations);

	auto state = detail::identity_measures(n);
	auto geometry_distances = source_distances;
	frames.push_back(state);
	for (std::size_t iteration = 0; iteration < options.iterations; ++iteration) {
		const auto op = detail::redif_operator_from_distances(geometry_distances, k, options.scale_policy);
		if (initial_stationary.empty()) {
			initial_stationary = op.stationary;
		}
		terminal_stationary = op.stationary;
		operator_diagnostics.push_back(op.diagnostics);
		RedifStabilityDiagnostics stability;
		const auto next = detail::redif_implicit_euler_step(op, state, options.euler_step, &stability,
															options.stability_tolerance,
															options.marginal_stability_tolerance);
		step_diagnostics.push_back(detail::summarize_state_entropy(iteration + 1, op, stability, next));
		step_transport.push_back(
			detail::transition_step_transport(state, next, transport, options, &transport_diagnostics));
		state = next;
		frames.push_back(state);
		if (options.adaptive_geometry) {
			geometry_distances = detail::transport_distance_matrix(state, transport);
		}
	}

	return detail::make_measure_result(ids, frames, step_transport, std::move(initial_stationary),
									   std::move(terminal_stationary), std::move(operator_diagnostics),
									   std::move(step_diagnostics), std::move(transport_diagnostics), k, options,
									   "redif_remove_noise",
									   options.adaptive_geometry ? "redif_inverse_implicit_euler_adaptive"
																 : "redif_inverse_implicit_euler_fixed",
									   "transport_measure_path");
}

inline auto redif_remove_noise_from_sampled_distance_matrix(
	const std::vector<std::vector<double>> &source_distances,
	redif_options options = {},
	redif_sampled_relation_options sample_options = {},
	std::vector<RecordId> ids = {}) -> RedifMeasureResult
{
	detail::validate_redif_options(source_distances.size(), options);
	detail::validate_redif_distance_matrix(source_distances);
	const std::size_t n = source_distances.size();
	const std::size_t k = std::min(options.neighbors, n > 0 ? n - 1 : 0);
	if (ids.empty()) {
		ids = redif_record_ids_from_count(n);
	}
	redif_validate_record_ids(ids, n);

	Wasserstein<double> transport(source_distances);
	auto transport_diagnostics = detail::make_redif_transport_diagnostics(n, options);
	std::vector<std::vector<std::vector<double>>> frames;
	std::vector<std::vector<double>> step_transport;
	std::vector<RedifOperatorDiagnostics> operator_diagnostics;
	std::vector<RedifStepDiagnostics> step_diagnostics;
	std::vector<double> initial_stationary;
	std::vector<double> terminal_stationary;
	frames.reserve(options.iterations + 1);
	step_transport.reserve(options.iterations);
	operator_diagnostics.reserve(options.iterations);
	step_diagnostics.reserve(options.iterations);

	auto state = detail::identity_measures(n);
	auto geometry_distances = source_distances;
	frames.push_back(state);
	for (std::size_t iteration = 0; iteration < options.iterations; ++iteration) {
		const auto op = detail::redif_operator_from_sampled_distances(
			geometry_distances, k, options.scale_policy, sample_options);
		if (initial_stationary.empty()) {
			initial_stationary = op.stationary;
		}
		terminal_stationary = op.stationary;
		operator_diagnostics.push_back(op.diagnostics);
		RedifStabilityDiagnostics stability;
		const auto next = detail::redif_implicit_euler_step(op, state, options.euler_step, &stability,
															options.stability_tolerance,
															options.marginal_stability_tolerance);
		step_diagnostics.push_back(detail::summarize_state_entropy(iteration + 1, op, stability, next));
		step_transport.push_back(
			detail::transition_step_transport(state, next, transport, options, &transport_diagnostics));
		state = next;
		frames.push_back(state);
		if (options.adaptive_geometry) {
			geometry_distances = detail::transport_distance_matrix(state, transport);
		}
	}

	return detail::make_measure_result(ids, frames, step_transport, std::move(initial_stationary),
									   std::move(terminal_stationary), std::move(operator_diagnostics),
									   std::move(step_diagnostics), std::move(transport_diagnostics), k, options,
									   "redif_remove_noise",
									   options.adaptive_geometry
										   ? "redif_inverse_implicit_euler_sampled_chunked_adaptive"
										   : "redif_inverse_implicit_euler_sampled_chunked_fixed",
									   "sampled_chunked_transport_measure_path");
}

inline auto redif_add_noise_from_distance_matrix(const std::vector<std::vector<double>> &source_distances,
												 redif_options options = {},
												 std::vector<RecordId> ids = {}) -> RedifMeasureResult
{
	detail::validate_redif_options(source_distances.size(), options);
	detail::validate_redif_distance_matrix(source_distances);
	const std::size_t n = source_distances.size();
	const std::size_t k = std::min(options.neighbors, n > 0 ? n - 1 : 0);
	if (ids.empty()) {
		ids = redif_record_ids_from_count(n);
	}
	redif_validate_record_ids(ids, n);

	Wasserstein<double> transport(source_distances);
	auto transport_diagnostics = detail::make_redif_transport_diagnostics(n, options);
	std::vector<std::vector<std::vector<double>>> frames;
	std::vector<std::vector<double>> step_transport;
	std::vector<RedifOperatorDiagnostics> operator_diagnostics;
	std::vector<RedifStepDiagnostics> step_diagnostics;
	std::vector<double> initial_stationary;
	std::vector<double> terminal_stationary;
	frames.reserve(options.iterations + 1);
	step_transport.reserve(options.iterations);
	operator_diagnostics.reserve(options.iterations);
	step_diagnostics.reserve(options.iterations);

	auto state = detail::identity_measures(n);
	auto geometry_distances = source_distances;
	frames.push_back(state);
	for (std::size_t iteration = 0; iteration < options.iterations; ++iteration) {
		const auto op = detail::redif_operator_from_distances(geometry_distances, k, options.scale_policy);
		if (initial_stationary.empty()) {
			initial_stationary = op.stationary;
		}
		terminal_stationary = op.stationary;
		operator_diagnostics.push_back(op.diagnostics);
		auto next = detail::redif_forward_euler_step(op, state, options.euler_step);
		step_diagnostics.push_back(detail::summarize_state_entropy(
			iteration + 1, op, detail::stable_step_diagnostics("forward_euler_probability_step"), next));
		step_transport.push_back(
			detail::transition_step_transport(state, next, transport, options, &transport_diagnostics));
		state = std::move(next);
		frames.push_back(state);
		if (options.adaptive_geometry) {
			geometry_distances = detail::transport_distance_matrix(state, transport);
		}
	}

	return detail::make_measure_result(ids, frames, step_transport, std::move(initial_stationary),
									   std::move(terminal_stationary), std::move(operator_diagnostics),
									   std::move(step_diagnostics), std::move(transport_diagnostics), k, options,
									   "redif_add_noise",
									   options.adaptive_geometry ? "redif_forward_euler_adaptive"
																 : "redif_forward_euler_fixed",
									   "transport_measure_path");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto redif_remove_noise(const Space &space, redif_options options = {}) -> RedifMeasureResult
{
	detail::validate_redif_options(space.size(), options);
	const std::size_t n = space.size();
	const std::size_t k = std::min(options.neighbors, n > 0 ? n - 1 : 0);

	const auto source_distances = detail::distance_matrix(space);
	Wasserstein<double> transport(source_distances);
	auto transport_diagnostics = detail::make_redif_transport_diagnostics(n, options);
	std::vector<RecordId> ids;
	ids.reserve(n);
	for (std::size_t i = 0; i < n; ++i) {
		ids.push_back(space.id(i));
	}

	std::vector<std::vector<std::vector<double>>> frames;
	std::vector<std::vector<double>> step_transport;
	std::vector<RedifOperatorDiagnostics> operator_diagnostics;
	std::vector<RedifStepDiagnostics> step_diagnostics;
	std::vector<double> initial_stationary;
	std::vector<double> terminal_stationary;
	frames.reserve(options.iterations + 1);
	step_transport.reserve(options.iterations);
	operator_diagnostics.reserve(options.iterations);
	step_diagnostics.reserve(options.iterations);

	auto state = detail::identity_measures(n);
	auto geometry_distances = source_distances;
	frames.push_back(state);

	for (std::size_t iteration = 0; iteration < options.iterations; ++iteration) {
		const auto op = detail::redif_operator_from_distances(geometry_distances, k, options.scale_policy);
		if (initial_stationary.empty()) {
			initial_stationary = op.stationary;
		}
		terminal_stationary = op.stationary;
		operator_diagnostics.push_back(op.diagnostics);
		RedifStabilityDiagnostics stability;
		const auto next = detail::redif_implicit_euler_step(op, state, options.euler_step, &stability,
															options.stability_tolerance,
															options.marginal_stability_tolerance);
		step_diagnostics.push_back(detail::summarize_state_entropy(iteration + 1, op, stability, next));

		step_transport.push_back(
			detail::transition_step_transport(state, next, transport, options, &transport_diagnostics));
		state = next;
		frames.push_back(state);
		if (options.adaptive_geometry) {
			geometry_distances = detail::transport_distance_matrix(state, transport);
		}
	}

	return detail::make_measure_result(ids, frames, step_transport, std::move(initial_stationary),
									   std::move(terminal_stationary), std::move(operator_diagnostics),
									   std::move(step_diagnostics), std::move(transport_diagnostics), k, options,
									   "redif_remove_noise",
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
	auto transport_diagnostics = detail::make_redif_transport_diagnostics(n, options);
	std::vector<RecordId> ids;
	ids.reserve(n);
	for (std::size_t i = 0; i < n; ++i) {
		ids.push_back(space.id(i));
	}

	std::vector<std::vector<std::vector<double>>> frames;
	std::vector<std::vector<double>> step_transport;
	std::vector<RedifOperatorDiagnostics> operator_diagnostics;
	std::vector<RedifStepDiagnostics> step_diagnostics;
	std::vector<double> initial_stationary;
	std::vector<double> terminal_stationary;
	frames.reserve(options.iterations + 1);
	step_transport.reserve(options.iterations);
	operator_diagnostics.reserve(options.iterations);
	step_diagnostics.reserve(options.iterations);
	auto state = detail::identity_measures(n);
	auto geometry_distances = source_distances;
	frames.push_back(state);
	for (std::size_t iteration = 0; iteration < options.iterations; ++iteration) {
		const auto op = detail::redif_operator_from_distances(geometry_distances, k, options.scale_policy);
		if (initial_stationary.empty()) {
			initial_stationary = op.stationary;
		}
		terminal_stationary = op.stationary;
		operator_diagnostics.push_back(op.diagnostics);
		auto next = detail::redif_forward_euler_step(op, state, options.euler_step);
		step_diagnostics.push_back(
			detail::summarize_state_entropy(iteration + 1, op, detail::stable_step_diagnostics("forward_euler_probability_step"), next));
		step_transport.push_back(
			detail::transition_step_transport(state, next, transport, options, &transport_diagnostics));
		state = std::move(next);
		frames.push_back(state);
		if (options.adaptive_geometry) {
			geometry_distances = detail::transport_distance_matrix(state, transport);
		}
	}

	return detail::make_measure_result(ids, frames, step_transport, std::move(initial_stationary),
									   std::move(terminal_stationary), std::move(operator_diagnostics),
									   std::move(step_diagnostics), std::move(transport_diagnostics), k, options,
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
auto redif_multiscale_transport_paths(const Space &space, std::vector<RedifScaleConfiguration> scales)
	-> RedifMultiscaleResult
{
	if (scales.empty()) {
		throw std::invalid_argument("Redif multiscale transport paths require at least one scale");
	}
	RedifMultiscaleResult result;
	result.scale_labels.reserve(scales.size());
	result.scores.reserve(space.size());
	for (std::size_t scale_index = 0; scale_index < scales.size(); ++scale_index) {
		auto &scale = scales[scale_index];
		if (scale.label.empty()) {
			scale.label = "scale_" + std::to_string(scale_index);
		}
		result.scale_labels.push_back(scale.label);
		const auto paths = redif_remove_noise(space, scale.options);
		const auto stability_weight = detail::redif_result_stability_weight(paths.step_diagnostics, scale.options);
		if (scale_index == 0) {
			for (const auto &path : paths.paths) {
				RedifMultiscaleScore score;
				score.id = path.id;
				score.scale_path_lengths.reserve(scales.size());
				score.scale_stability_weights.reserve(scales.size());
				result.scores.push_back(std::move(score));
			}
		}
		if (paths.size() != result.scores.size()) {
			throw std::runtime_error("Redif multiscale runs changed the atom count");
		}
		for (std::size_t record = 0; record < paths.size(); ++record) {
			if (result.scores[record].id != paths.paths[record].id) {
				throw std::runtime_error("Redif multiscale runs changed atom identity order");
			}
			result.scores[record].scale_path_lengths.push_back(paths.paths[record].transport_path_length);
			result.scores[record].scale_stability_weights.push_back(stability_weight);
		}
		result.exact = result.exact && paths.exact;
	}

	for (auto &score : result.scores) {
		auto sorted = score.scale_path_lengths;
		std::sort(sorted.begin(), sorted.end());
		score.median_path_length = sorted.empty() ? 0.0 : sorted[sorted.size() / 2];
		score.maximum_path_length = sorted.empty() ? 0.0 : sorted.back();
		double weighted_total = 0.0;
		double weight_total = 0.0;
		for (std::size_t index = 0; index < score.scale_path_lengths.size(); ++index) {
			const auto weight = score.scale_stability_weights[index];
			weighted_total += score.scale_path_lengths[index] * weight;
			weight_total += weight;
		}
		score.stability_weighted_path_length =
			weight_total > 0.0 ? weighted_total / weight_total : score.median_path_length;
	}
	std::sort(result.scores.begin(), result.scores.end(), [](const auto &lhs, const auto &rhs) {
		if (lhs.median_path_length != rhs.median_path_length) {
			return lhs.median_path_length > rhs.median_path_length;
		}
		return lhs.id.index() < rhs.id.index();
	});
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
using RedifScaleConfiguration = modify::dynamics::RedifScaleConfiguration;
using RedifEntropyDiagnostics = modify::dynamics::RedifEntropyDiagnostics;
using RedifLocalRelationDiagnostics = modify::dynamics::RedifLocalRelationDiagnostics;
using RedifLocalRelationEntry = modify::dynamics::RedifLocalRelationEntry;
using RedifMeasurePath = modify::dynamics::RedifMeasurePath;
using RedifMeasureResult = modify::dynamics::RedifMeasureResult;
using RedifMeasureSummary = modify::dynamics::RedifMeasureSummary;
using RedifMultiscaleResult = modify::dynamics::RedifMultiscaleResult;
using RedifMultiscaleScore = modify::dynamics::RedifMultiscaleScore;
using RedifSparseEntry = modify::dynamics::RedifSparseEntry;
using RedifSparseOperator = modify::dynamics::RedifSparseOperator;
using RedifStabilityDiagnostics = modify::dynamics::RedifStabilityDiagnostics;
using RedifStepDiagnostics = modify::dynamics::RedifStepDiagnostics;
using RedifTransportDiagnostics = modify::dynamics::RedifTransportDiagnostics;
using EntropyTrajectory = modify::dynamics::EntropyTrajectory;
using redif_options = modify::dynamics::redif_options;
using redif_sampled_relation_options = modify::dynamics::redif_sampled_relation_options;
using redif_scale_policy = modify::dynamics::redif_scale_policy;
using modify::dynamics::entropy_trajectory;
using modify::dynamics::redif_add_noise;
using modify::dynamics::redif_add_noise_from_distance_matrix;
using modify::dynamics::redif_forward_noise_step;
using modify::dynamics::redif_inverse_noise_step;
using modify::dynamics::redif_local_relation_refusal;
using modify::dynamics::redif_multiscale_transport_paths;
using modify::dynamics::redif_operator;
using modify::dynamics::redif_operator_from_distance_matrix;
using modify::dynamics::redif_operator_from_exact_neighbor_provider;
using modify::dynamics::redif_operator_from_sampled_distance_matrix;
using modify::dynamics::redif_operator_from_sparse_operator;
using modify::dynamics::redif_outliers;
using modify::dynamics::redif_record_ids_from_count;
using modify::dynamics::redif_remove_noise;
using modify::dynamics::redif_remove_noise_from_distance_matrix;
using modify::dynamics::redif_remove_noise_from_sampled_distance_matrix;
using modify::dynamics::redif_sparse_operator_from_exact_local_relation;
using modify::dynamics::redif_sparse_operator_from_exact_neighbor_provider;
} // namespace mtrc

#endif
