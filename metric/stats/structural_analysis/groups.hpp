// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_STRUCTURAL_ANALYSIS_GROUPS_HPP
#define _METRIC_STATS_STRUCTURAL_ANALYSIS_GROUPS_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/stats/structural_analysis/clustering.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/stats/structural_analysis/options.hpp>

namespace mtrc::stats::structural_analysis {
namespace group_detail {

constexpr std::size_t default_approximate_group_sample_count = 512;
constexpr std::size_t max_default_exact_structural_records = 4096;

inline auto approximate_sample_count(std::size_t record_count, std::size_t requested_count,
									 space::storage::policy runtime_policy) -> std::size_t
{
	if (record_count == 0) {
		return 0;
	}
	if (runtime_policy.graph_neighbors() != 0) {
		return std::min(runtime_policy.graph_neighbors(), record_count);
	}
	auto sample_count = std::min(default_approximate_group_sample_count, record_count);
	if (requested_count > sample_count) {
		sample_count = std::min(requested_count, record_count);
	}
	return sample_count;
}

inline auto should_use_default_approximation(std::size_t record_count) -> bool
{
	return record_count > max_default_exact_structural_records;
}

inline auto regular_sample_positions(std::size_t record_count, std::size_t sample_count) -> std::vector<std::size_t>
{
	std::vector<std::size_t> positions;
	if (record_count == 0 || sample_count == 0) {
		return positions;
	}
	sample_count = std::min(sample_count, record_count);
	positions.reserve(sample_count);
	for (std::size_t sample = 0; sample < sample_count; ++sample) {
		const auto position = (sample * record_count) / sample_count;
		if (positions.empty() || positions.back() != position) {
			positions.push_back(position);
		}
	}
	return positions;
}

template <typename Provider, typename = void> struct HasDistanceAtPosition : std::false_type {};

template <typename Provider>
struct HasDistanceAtPosition<
	Provider, std::void_t<decltype(std::declval<const Provider &>().distance_at_position(
				  std::declval<std::size_t>(), std::declval<std::size_t>()))>> : std::true_type {};

template <typename Provider, typename = void> struct HasGuardedDistanceAtPosition : std::false_type {};

template <typename Provider>
struct HasGuardedDistanceAtPosition<
	Provider, std::void_t<decltype(std::declval<const Provider &>().distance_at_position(
				  std::declval<std::size_t>(), std::declval<std::size_t>(),
				  std::declval<space::storage::runtime_guard>()))>> : std::true_type {};

template <typename Provider>
auto guarded_distance(const Provider &provider, RecordId lhs, RecordId rhs,
					  space::storage::runtime_guard runtime, const char *operation)
	-> typename Provider::distance_type
{
	runtime.throw_if_cancelled(operation);
	return provider.distance(lhs, rhs);
}

template <typename Provider>
auto guarded_distance_at_position(const Provider &provider, std::size_t lhs_position, std::size_t rhs_position,
								  space::storage::runtime_guard runtime, const char *operation)
	-> typename Provider::distance_type
{
	runtime.throw_if_cancelled(operation);
	if constexpr (HasGuardedDistanceAtPosition<Provider>::value) {
		return provider.distance_at_position(lhs_position, rhs_position, runtime);
	} else if constexpr (HasDistanceAtPosition<Provider>::value) {
		return provider.distance_at_position(lhs_position, rhs_position);
	} else {
		return provider.distance(provider.id(lhs_position), provider.id(rhs_position));
	}
}

template <typename Provider>
auto guarded_total_distance_to_positions(const Provider &provider, std::size_t source_position,
										 const std::vector<std::size_t> &record_positions,
										 space::storage::runtime_guard runtime,
										 const char *operation) -> typename Provider::distance_type
{
	typename Provider::distance_type total{};
	for (const auto position : record_positions) {
		total += guarded_distance_at_position(provider, source_position, position, runtime, operation);
	}
	return total;
}

template <typename Provider>
auto guarded_minimum_total_distance_record_id(const Provider &provider, const std::vector<RecordId> &record_ids,
											  const char *empty_message, const char *not_found_message,
											  space::storage::runtime_guard runtime,
											  const char *operation) -> RecordId
{
	if (record_ids.empty()) {
		throw std::invalid_argument(empty_message);
	}
	for (const auto id : record_ids) {
		if (!provider.contains(id)) {
			throw std::invalid_argument(not_found_message);
		}
	}

	std::vector<std::size_t> record_positions;
	record_positions.reserve(record_ids.size());
	for (const auto id : record_ids) {
		record_positions.push_back(provider.position_of(id));
	}

	auto best_id = record_ids.front();
	auto best_total = guarded_total_distance_to_positions(provider, record_positions.front(),
														  record_positions, runtime, operation);
	for (std::size_t index = 1; index < record_ids.size(); ++index) {
		const auto candidate = record_ids[index];
		const auto total = guarded_total_distance_to_positions(provider, record_positions[index],
															   record_positions, runtime, operation);
		if (total < best_total || (total == best_total && candidate.index() < best_id.index())) {
			best_id = candidate;
			best_total = total;
		}
	}
	return best_id;
}

template <typename Provider>
auto guarded_total_distance_to_provider_records(const Provider &provider, RecordId source_id,
												space::storage::runtime_guard runtime,
												const char *operation) -> typename Provider::distance_type
{
	const auto source_position = provider.position_of(source_id);
	typename Provider::distance_type total{};
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		total += guarded_distance_at_position(provider, source_position, index, runtime, operation);
	}
	return total;
}

template <typename Provider>
auto guarded_distances_to_record_id(const Provider &provider, RecordId target_id,
									const char *not_found_message, space::storage::runtime_guard runtime,
									const char *operation) -> std::vector<typename Provider::distance_type>
{
	if (!provider.contains(target_id)) {
		throw std::invalid_argument(not_found_message);
	}

	const auto target_position = provider.position_of(target_id);
	std::vector<typename Provider::distance_type> distances;
	distances.reserve(provider.record_count());
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		distances.push_back(guarded_distance_at_position(provider, index, target_position, runtime, operation));
	}
	return distances;
}

template <typename Provider>
auto guarded_update_min_distances_to_record_id(const Provider &provider,
											   std::vector<typename Provider::distance_type> &distances,
											   RecordId target_id, const char *size_message,
											   const char *not_found_message,
											   space::storage::runtime_guard runtime,
											   const char *operation) -> void
{
	if (distances.size() != provider.record_count()) {
		throw std::invalid_argument(size_message);
	}
	if (!provider.contains(target_id)) {
		throw std::invalid_argument(not_found_message);
	}

	const auto target_position = provider.position_of(target_id);
	for (std::size_t index = 0; index < distances.size(); ++index) {
		const auto distance = guarded_distance_at_position(provider, index, target_position, runtime, operation);
		if (distance < distances[index]) {
			distances[index] = distance;
		}
	}
}

template <typename Provider>
auto guarded_initialize_medoids(const Provider &provider, std::size_t cluster_count,
								space::storage::runtime_guard runtime) -> std::vector<RecordId>
{
	using distance_type = typename Provider::distance_type;

	std::vector<RecordId> medoids;
	medoids.reserve(cluster_count);

	RecordId first_medoid;
	distance_type best_total{};
	bool has_best = false;
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		const auto candidate = provider.id(index);
		const auto total = guarded_total_distance_to_provider_records(
			provider, candidate, runtime, "k-medoids medoid initialization");
		if (!has_best || total < best_total) {
			first_medoid = candidate;
			best_total = total;
			has_best = true;
		}
	}
	medoids.push_back(first_medoid);

	std::vector<bool> is_selected(provider.record_count(), false);
	is_selected[provider.position_of(first_medoid)] = true;
	auto nearest_medoid_distances = guarded_distances_to_record_id(
		provider, first_medoid, "k-medoids medoid id is outside provider",
		runtime, "k-medoids medoid initialization");

	while (medoids.size() < cluster_count) {
		runtime.throw_if_cancelled("k-medoids medoid initialization");
		const auto next_position = core::farthest_unselected_record_position(
			provider, nearest_medoid_distances, is_selected,
			"selected medoid count does not match provider distance count", "failed to initialize k-medoids");
		const auto next_medoid = provider.id(next_position);
		medoids.push_back(next_medoid);
		is_selected[next_position] = true;
		guarded_update_min_distances_to_record_id(
			provider, nearest_medoid_distances, next_medoid,
			"nearest medoid distance count does not match provider",
			"k-medoids medoid id is outside provider", runtime, "k-medoids medoid initialization");
	}

	std::sort(medoids.begin(), medoids.end());
	return medoids;
}

template <typename Provider>
auto guarded_assign_to_medoids(const Provider &provider, const std::vector<RecordId> &medoids,
							   space::storage::runtime_guard runtime,
							   const char *operation)
	-> std::pair<std::vector<std::size_t>, std::vector<std::size_t>>
{
	if (medoids.empty()) {
		throw std::invalid_argument("column_argmin_indices requires at least one row");
	}

	std::vector<std::size_t> medoid_positions;
	medoid_positions.reserve(medoids.size());
	for (const auto medoid : medoids) {
		medoid_positions.push_back(provider.position_of(medoid));
	}

	std::vector<std::size_t> assignments(provider.record_count(), 0);
	for (std::size_t position = 0; position < provider.record_count(); ++position) {
		auto best_distance = guarded_distance_at_position(provider, position, medoid_positions.front(), runtime,
														  operation);
		for (std::size_t cluster = 1; cluster < medoids.size(); ++cluster) {
			const auto distance = guarded_distance_at_position(provider, position, medoid_positions[cluster],
															   runtime, operation);
			if (distance < best_distance) {
				best_distance = distance;
				assignments[position] = cluster;
			}
		}
	}

	auto cluster_sizes = numeric::index_counts(assignments, medoids.size());
	return {assignments, cluster_sizes};
}

template <typename Provider>
auto guarded_recompute_medoids(const Provider &provider, const std::vector<std::size_t> &assignments,
							   const std::vector<RecordId> &current_medoids,
							   space::storage::runtime_guard runtime) -> std::vector<RecordId>
{
	std::vector<RecordId> medoids;
	medoids.reserve(current_medoids.size());

	const auto ids_by_cluster = core::record_id_buckets_for_assignments(
		provider, assignments, current_medoids.size(), "k-medoids assignment references an unknown cluster");
	for (std::size_t cluster = 0; cluster < ids_by_cluster.size(); ++cluster) {
		runtime.throw_if_cancelled("k-medoids medoid recompute");
		if (ids_by_cluster[cluster].empty()) {
			medoids.push_back(current_medoids[cluster]);
			continue;
		}
		const auto &cluster_ids = ids_by_cluster[cluster];
		medoids.push_back(guarded_minimum_total_distance_record_id(
			provider, cluster_ids, "k-medoids cluster must not be empty",
			"k-medoids cluster id is outside provider", runtime, "k-medoids medoid recompute"));
	}

	std::sort(medoids.begin(), medoids.end());
	return medoids;
}

template <typename Provider>
auto guarded_compute_cluster_medoids(const Provider &provider, const std::vector<std::size_t> &assignments,
									 std::size_t cluster_count, space::storage::runtime_guard runtime)
	-> std::vector<RecordId>
{
	using distance_type = typename Provider::distance_type;
	const auto unassigned_label = ClusteringResult<distance_type>::unassigned_label;

	std::vector<RecordId> medoids;
	medoids.reserve(cluster_count);

	const auto ids_by_cluster = core::record_id_buckets_excluding_assignment(
		provider, assignments, cluster_count, unassigned_label, "clustering assignment references an unknown cluster");
	for (const auto &cluster_ids : ids_by_cluster) {
		runtime.throw_if_cancelled("dbscan medoid computation");
		if (!cluster_ids.empty()) {
			medoids.push_back(guarded_minimum_total_distance_record_id(
				provider, cluster_ids, "cluster must not be empty", "cluster id is outside provider",
				runtime, "dbscan medoid computation"));
		}
	}

	return medoids;
}

template <typename Provider>
auto guarded_kmedoids(const Provider &provider, stats::structural_analysis::k_medoids_options strategy,
					  space::storage::runtime_guard runtime) -> ClusteringResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot cluster an empty distance provider");
	}
	if (strategy.groups == 0) {
		throw std::invalid_argument("cluster_count must be positive");
	}
	if (strategy.groups > provider.record_count()) {
		throw std::invalid_argument("cluster_count cannot exceed record_count");
	}

	auto medoids = guarded_initialize_medoids(provider, strategy.groups, runtime);
	auto assignment_state = guarded_assign_to_medoids(provider, medoids, runtime, "k-medoids assignment");
	auto assignments = std::move(assignment_state.first);
	auto cluster_sizes = std::move(assignment_state.second);

	std::size_t iterations = 0;
	bool converged = false;
	for (; iterations < strategy.max_iterations; ++iterations) {
		runtime.throw_if_cancelled("k-medoids iteration");
		auto updated_medoids = guarded_recompute_medoids(provider, assignments, medoids, runtime);
		auto updated_assignment_state =
			guarded_assign_to_medoids(provider, updated_medoids, runtime, "k-medoids assignment");
		auto updated_assignments = std::move(updated_assignment_state.first);
		auto updated_cluster_sizes = std::move(updated_assignment_state.second);

		if (updated_medoids == medoids && updated_assignments == assignments) {
			converged = true;
			medoids = std::move(updated_medoids);
			assignments = std::move(updated_assignments);
			cluster_sizes = std::move(updated_cluster_sizes);
			++iterations;
			break;
		}

		medoids = std::move(updated_medoids);
		assignments = std::move(updated_assignments);
		cluster_sizes = std::move(updated_cluster_sizes);
	}

	return core::make_clustering_result<distance_type>(std::move(assignments), std::move(medoids), {}, {},
													   std::move(cluster_sizes), iterations, converged, "kmedoids",
													   "pairwise_distances");
}

template <typename Space>
auto guarded_kmedoids_space(const Space &space, stats::structural_analysis::k_medoids_options strategy,
							space::storage::runtime_guard runtime)
	-> ClusteringResult<typename Space::distance_type>
{
	engine_detail::require_default_exact_metric_space_clustering(space.size());
	space::storage::LiveDistances<Space> provider(space);
	auto result = guarded_kmedoids(provider, strategy, runtime);
	result.representation = "metric_space";
	return result;
}

template <typename Provider, typename Radius>
auto guarded_dbscan_region_query(const Provider &provider, std::size_t query_position, Radius radius,
								 space::storage::runtime_guard runtime) -> std::vector<std::size_t>
{
	std::vector<std::size_t> neighbors;
	neighbors.reserve(provider.record_count());
	for (std::size_t candidate_position = 0; candidate_position < provider.record_count(); ++candidate_position) {
		if (guarded_distance_at_position(provider, query_position, candidate_position, runtime,
										 "dbscan region query") <= radius) {
			neighbors.push_back(candidate_position);
		}
	}
	return neighbors;
}

template <typename Provider, typename Radius>
auto guarded_expand_dbscan_cluster(const Provider &provider, const std::vector<std::size_t> &seed_neighbors,
								   Radius radius, std::size_t min_points, std::size_t cluster,
								   std::vector<bool> &visited, std::vector<bool> &assigned,
								   std::vector<std::size_t> &assignments, std::vector<bool> &core_records,
								   space::storage::runtime_guard runtime) -> void
{
	std::vector<std::size_t> queue;
	std::vector<bool> queued(provider.record_count(), false);
	queue.reserve(seed_neighbors.size());

	for (const auto neighbor_position : seed_neighbors) {
		queue.push_back(neighbor_position);
		queued[neighbor_position] = true;
	}

	for (std::size_t cursor = 0; cursor < queue.size(); ++cursor) {
		runtime.throw_if_cancelled("dbscan cluster expansion");
		const auto candidate_index = queue[cursor];

		if (!visited[candidate_index]) {
			visited[candidate_index] = true;
			auto neighbors = guarded_dbscan_region_query(provider, candidate_index, radius, runtime);
			if (neighbors.size() >= min_points) {
				core_records[candidate_index] = true;
				for (const auto neighbor_position : neighbors) {
					if (!queued[neighbor_position]) {
						queue.push_back(neighbor_position);
						queued[neighbor_position] = true;
					}
				}
			}
		}

		if (!assigned[candidate_index]) {
			assignments[candidate_index] = cluster;
			assigned[candidate_index] = true;
		}
	}
}

template <typename Provider, typename Radius>
auto guarded_dbscan(const Provider &provider, Radius radius, std::size_t min_points,
					space::storage::runtime_guard runtime) -> ClusteringResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	const auto unassigned_label = ClusteringResult<distance_type>::unassigned_label;

	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot cluster an empty distance provider");
	}
	engine_detail::validate_dbscan_parameters(radius, min_points);

	std::vector<std::size_t> assignments(provider.record_count(), unassigned_label);
	std::vector<bool> assigned(provider.record_count(), false);
	std::vector<bool> visited(provider.record_count(), false);
	std::vector<bool> core_record_flags(provider.record_count(), false);

	std::size_t cluster_count = 0;
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		runtime.throw_if_cancelled("dbscan clustering");
		if (visited[index]) {
			continue;
		}

		visited[index] = true;
		auto neighbors = guarded_dbscan_region_query(provider, index, radius, runtime);
		if (neighbors.size() < min_points) {
			continue;
		}

		core_record_flags[index] = true;
		guarded_expand_dbscan_cluster(provider, neighbors, radius, min_points, cluster_count, visited, assigned,
									  assignments, core_record_flags, runtime);
		++cluster_count;
	}

	auto unassigned_records = core::record_ids_matching_value(provider, assignments, unassigned_label);
	auto core_records = core::record_ids_matching_value(provider, core_record_flags, true);
	auto cluster_sizes = numeric::index_counts_excluding(assignments, cluster_count, unassigned_label,
														"dbscan assignment references an unknown cluster");
	auto medoids = guarded_compute_cluster_medoids(provider, assignments, cluster_count, runtime);

	return core::make_clustering_result<distance_type>(std::move(assignments), std::move(medoids),
													   std::move(core_records), std::move(unassigned_records),
													   std::move(cluster_sizes), 1, true, "dbscan",
													   "pairwise_distances");
}

template <typename Space, typename Radius>
auto guarded_dbscan_space(const Space &space, Radius radius, std::size_t min_points,
						  space::storage::runtime_guard runtime)
	-> ClusteringResult<typename Space::distance_type>
{
	engine_detail::require_default_exact_metric_space_clustering(space.size());
	space::storage::LiveDistances<Space> provider(space);
	auto result = guarded_dbscan(provider, radius, min_points, runtime);
	result.representation = "metric_space";
	return result;
}

template <typename Provider>
auto sampled_kmedoids(const Provider &provider, stats::structural_analysis::k_medoids_options strategy,
					  space::storage::policy runtime_policy,
					  space::storage::runtime_guard runtime = {})
	-> ClusteringResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot cluster an empty distance provider");
	}
	if (strategy.groups == 0) {
		throw std::invalid_argument("cluster_count must be positive");
	}
	if (strategy.groups > provider.record_count()) {
		throw std::invalid_argument("cluster_count cannot exceed record_count");
	}

	const auto sample_count = approximate_sample_count(provider.record_count(), strategy.groups, runtime_policy);
	const auto positions = regular_sample_positions(provider.record_count(), sample_count);
	std::vector<RecordId> medoids;
	medoids.reserve(strategy.groups);
	for (const auto position : positions) {
		medoids.push_back(provider.id(position));
		if (medoids.size() == strategy.groups) {
			break;
		}
	}
	while (medoids.size() < strategy.groups) {
		medoids.push_back(provider.id(medoids.size()));
	}

	auto assignment_state = guarded_assign_to_medoids(
		provider, medoids, runtime, "sampled k-medoids assignment");
	auto result = core::make_clustering_result<distance_type>(
		std::move(assignment_state.first), std::move(medoids), {}, {}, std::move(assignment_state.second), 1, true,
		"sampled_kmedoids", "sampled_metric_space", false);
	return result;
}

inline auto find_root(std::vector<std::size_t> &parents, std::size_t index) -> std::size_t
{
	while (parents[index] != index) {
		parents[index] = parents[parents[index]];
		index = parents[index];
	}
	return index;
}

inline auto unite_roots(std::vector<std::size_t> &parents, std::size_t lhs, std::size_t rhs) -> void
{
	const auto lhs_root = find_root(parents, lhs);
	const auto rhs_root = find_root(parents, rhs);
	if (lhs_root != rhs_root) {
		parents[rhs_root] = lhs_root;
	}
}

template <typename Provider, typename Radius>
auto sampled_dbscan(const Provider &provider, Radius radius, std::size_t min_points,
					space::storage::policy runtime_policy,
					space::storage::runtime_guard runtime = {})
	-> ClusteringResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	const auto unassigned_label = ClusteringResult<distance_type>::unassigned_label;
	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot cluster an empty distance provider");
	}
	engine_detail::validate_dbscan_parameters(radius, min_points);

	const auto sample_count = approximate_sample_count(provider.record_count(), min_points, runtime_policy);
	const auto positions = regular_sample_positions(provider.record_count(), sample_count);
	std::vector<RecordId> sample_ids;
	sample_ids.reserve(positions.size());
	for (const auto position : positions) {
		sample_ids.push_back(provider.id(position));
	}

	std::vector<bool> core_sample(sample_ids.size(), false);
	for (std::size_t sample = 0; sample < sample_ids.size(); ++sample) {
		runtime.throw_if_cancelled("sampled dbscan core detection");
		std::size_t within_radius = 0;
		for (const auto candidate_position : positions) {
			if (guarded_distance_at_position(provider, positions[sample], candidate_position, runtime,
											 "sampled dbscan core detection") <= radius) {
				++within_radius;
			}
		}
		core_sample[sample] = within_radius >= min_points;
	}

	std::vector<std::size_t> parents(sample_ids.size());
	for (std::size_t index = 0; index < parents.size(); ++index) {
		parents[index] = index;
	}
	for (std::size_t lhs = 0; lhs < sample_ids.size(); ++lhs) {
		runtime.throw_if_cancelled("sampled dbscan core union");
		if (!core_sample[lhs]) {
			continue;
		}
		for (std::size_t rhs = lhs + 1; rhs < sample_ids.size(); ++rhs) {
			if (core_sample[rhs] &&
				guarded_distance_at_position(provider, positions[lhs], positions[rhs], runtime,
											 "sampled dbscan core union") <= radius) {
				unite_roots(parents, lhs, rhs);
			}
		}
	}

	std::vector<std::size_t> root_labels(sample_ids.size(), unassigned_label);
	std::vector<RecordId> medoids;
	std::vector<RecordId> core_records;
	std::vector<std::size_t> core_indices;
	std::vector<std::size_t> cluster_sizes;
	for (std::size_t sample = 0; sample < sample_ids.size(); ++sample) {
		if (!core_sample[sample]) {
			continue;
		}
		core_indices.push_back(sample);
		core_records.push_back(sample_ids[sample]);
		const auto root = find_root(parents, sample);
		if (root_labels[root] == unassigned_label) {
			root_labels[root] = cluster_sizes.size();
			cluster_sizes.push_back(0);
			medoids.push_back(sample_ids[sample]);
		}
	}

	std::vector<std::size_t> assignments(provider.record_count(), unassigned_label);
	for (std::size_t position = 0; position < provider.record_count(); ++position) {
		runtime.throw_if_cancelled("sampled dbscan assignment");
		std::size_t best_label = unassigned_label;
		distance_type best_distance{};
		bool has_best = false;
		for (const auto core_index : core_indices) {
			const auto distance = guarded_distance_at_position(provider, position, positions[core_index],
															   runtime, "sampled dbscan assignment");
			if (distance > radius) {
				continue;
			}
			if (!has_best || distance < best_distance) {
				best_distance = distance;
				best_label = root_labels[find_root(parents, core_index)];
				has_best = true;
			}
		}
		if (has_best && best_label != unassigned_label) {
			assignments[position] = best_label;
			++cluster_sizes[best_label];
		}
	}

	auto unassigned_records = core::record_ids_matching_value(provider, assignments, unassigned_label);
	return core::make_clustering_result<distance_type>(
		std::move(assignments), std::move(medoids), std::move(core_records), std::move(unassigned_records),
		std::move(cluster_sizes), 1, true, "sampled_dbscan", "sampled_metric_space", false);
}

} // namespace group_detail

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto find_groups(const Provider &provider, stats::structural_analysis::k_medoids_options strategy)
	-> ClusteringResult<typename Provider::distance_type>
{
	return group_detail::guarded_kmedoids(provider, strategy, {});
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, stats::structural_analysis::k_medoids_options strategy) -> ClusteringResult<typename Space::distance_type>
{
	if (group_detail::should_use_default_approximation(space.size())) {
		space::storage::LiveDistances<Space> provider(space);
		return group_detail::sampled_kmedoids(provider, strategy, space::storage::approximate());
	}
	return stats::structural_analysis::kmedoids(space, strategy.groups, strategy.max_iterations);
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_groups(const Container &records, const Metric &metric, stats::structural_analysis::k_medoids_options strategy)
	-> ClusteringResult<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = find_groups(space, strategy);
	result.representation = "records";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, stats::structural_analysis::k_medoids_options strategy, space::storage::policy runtime_policy)
	-> ClusteringResult<typename Space::distance_type>
{
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.is_approximate()) {
		auto runtime = space::storage::runtime_guard(runtime_policy);
		space::storage::LiveDistances<Space> provider(space);
		return group_detail::sampled_kmedoids(provider, strategy, runtime_policy, runtime);
	}
	space::storage::require_exact_groups(runtime_policy);
	auto runtime = space::storage::runtime_guard(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		return space::storage::with_materialized_distance_provider(
			space, runtime_policy, "groups", [strategy, runtime](const auto &provider, const auto &plan) {
				auto result = group_detail::guarded_kmedoids(provider, strategy, runtime);
				result.representation = space::storage::materialized_operator_representation(plan);
				return result;
			});
	}

	auto result = group_detail::guarded_kmedoids_space(space, strategy, runtime);
	result.representation = space::storage::group_representation(runtime_policy);
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_groups(const Container &records, const Metric &metric,
				 stats::structural_analysis::k_medoids_options strategy, space::storage::policy runtime_policy)
	-> ClusteringResult<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	return find_groups(space, strategy, runtime_policy);
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto find_groups(const Provider &provider, std::size_t groups) -> ClusteringResult<typename Provider::distance_type>
{
	return find_groups(provider, stats::structural_analysis::k_medoids_options(groups));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, std::size_t groups) -> ClusteringResult<typename Space::distance_type>
{
	return find_groups(space, stats::structural_analysis::k_medoids_options(groups));
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_groups(const Container &records, const Metric &metric, std::size_t groups)
	-> ClusteringResult<metric_result_t<Metric, Record>>
{
	return find_groups(records, metric, stats::structural_analysis::k_medoids_options(groups));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, std::size_t groups, space::storage::policy runtime_policy)
	-> ClusteringResult<typename Space::distance_type>
{
	return find_groups(space, stats::structural_analysis::k_medoids_options(groups), runtime_policy);
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_groups(const Container &records, const Metric &metric, std::size_t groups,
				 space::storage::policy runtime_policy) -> ClusteringResult<metric_result_t<Metric, Record>>
{
	return find_groups(records, metric, stats::structural_analysis::k_medoids_options(groups), runtime_policy);
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto find_groups(const Provider &provider, stats::structural_analysis::dbscan_options strategy)
	-> ClusteringResult<typename Provider::distance_type>
{
	return group_detail::guarded_dbscan(provider, strategy.radius, strategy.min_points, {});
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, stats::structural_analysis::dbscan_options strategy) -> ClusteringResult<typename Space::distance_type>
{
	if (group_detail::should_use_default_approximation(space.size())) {
		space::storage::LiveDistances<Space> provider(space);
		return group_detail::sampled_dbscan(provider, strategy.radius, strategy.min_points, space::storage::approximate());
	}
	return stats::structural_analysis::dbscan(space, strategy.radius, strategy.min_points);
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_groups(const Container &records, const Metric &metric, stats::structural_analysis::dbscan_options strategy)
	-> ClusteringResult<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = find_groups(space, strategy);
	result.representation = "records";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, stats::structural_analysis::dbscan_options strategy, space::storage::policy runtime_policy)
	-> ClusteringResult<typename Space::distance_type>
{
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.is_approximate()) {
		auto runtime = space::storage::runtime_guard(runtime_policy);
		space::storage::LiveDistances<Space> provider(space);
		return group_detail::sampled_dbscan(provider, strategy.radius, strategy.min_points, runtime_policy, runtime);
	}
	space::storage::require_exact_groups(runtime_policy);
	auto runtime = space::storage::runtime_guard(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		return space::storage::with_materialized_distance_provider(
			space, runtime_policy, "groups", [strategy, runtime](const auto &provider, const auto &plan) {
				auto result = group_detail::guarded_dbscan(provider, strategy.radius, strategy.min_points, runtime);
				result.representation = space::storage::materialized_operator_representation(plan);
				return result;
			});
	}

	auto result = group_detail::guarded_dbscan_space(space, strategy.radius, strategy.min_points, runtime);
	result.representation = space::storage::group_representation(runtime_policy);
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_groups(const Container &records, const Metric &metric,
				 stats::structural_analysis::dbscan_options strategy, space::storage::policy runtime_policy)
	-> ClusteringResult<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	return find_groups(space, strategy, runtime_policy);
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto find_groups(const Provider &provider, stats::structural_analysis::affinity_propagation_options strategy)
	-> ClusteringResult<typename Provider::distance_type>
{
	return stats::structural_analysis::affinity_propagation(provider, strategy.preference, strategy.max_iterations,
														   strategy.tolerance, strategy.damping);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, stats::structural_analysis::affinity_propagation_options strategy)
	-> ClusteringResult<typename Space::distance_type>
{
	if (group_detail::should_use_default_approximation(space.size())) {
		throw RepresentationError(
			"affinity propagation default execution exceeds the large-data exact threshold; "
			"use an explicit exact runtime policy for small data or choose sampled k-medoids/DBSCAN for large data");
	}
	return stats::structural_analysis::affinity_propagation(space, strategy.preference, strategy.max_iterations, strategy.tolerance,
										   strategy.damping);
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_groups(const Container &records, const Metric &metric,
				 stats::structural_analysis::affinity_propagation_options strategy)
	-> ClusteringResult<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = find_groups(space, strategy);
	result.representation = "records";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, stats::structural_analysis::affinity_propagation_options strategy, space::storage::policy runtime_policy)
	-> ClusteringResult<typename Space::distance_type>
{
	space::storage::require_exact_groups(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		return space::storage::with_materialized_distance_provider(
			space, runtime_policy, "groups", [&](const auto &provider, const auto &plan) {
				auto result = stats::structural_analysis::affinity_propagation(
					provider, strategy.preference, strategy.max_iterations, strategy.tolerance, strategy.damping);
				result.representation = space::storage::materialized_operator_representation(plan);
				return result;
			});
	}

	auto result = stats::structural_analysis::affinity_propagation(space, strategy.preference, strategy.max_iterations,
												  strategy.tolerance, strategy.damping);
	result.representation = space::storage::group_representation(runtime_policy);
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_groups(const Container &records, const Metric &metric,
				 stats::structural_analysis::affinity_propagation_options strategy,
				 space::storage::policy runtime_policy) -> ClusteringResult<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	return find_groups(space, strategy, runtime_policy);
}

} // namespace mtrc::stats::structural_analysis

namespace mtrc {
using stats::structural_analysis::find_groups;
} // namespace mtrc

#endif
