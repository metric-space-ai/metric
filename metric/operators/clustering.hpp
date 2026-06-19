// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_CLUSTERING_HPP
#define _METRIC_OPERATORS_CLUSTERING_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "../core/concepts.hpp"
#include "../core/record_id.hpp"
#include "../core/result.hpp"
#include "../representations/implicit.hpp"

namespace metric::operators {

namespace engine_detail {

template <typename Provider>
auto total_distance_to_all(const Provider &provider, RecordId candidate) -> typename Provider::distance_type
{
	using distance_type = typename Provider::distance_type;

	distance_type total{};
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		total += provider.distance(candidate, RecordId::from_index(index));
	}
	return total;
}

auto contains_medoid(const std::vector<RecordId> &medoids, RecordId id) -> bool
{
	return std::find(medoids.begin(), medoids.end(), id) != medoids.end();
}

template <typename Provider>
auto nearest_medoid_distance(const Provider &provider, const std::vector<RecordId> &medoids, RecordId id)
	-> typename Provider::distance_type
{
	using distance_type = typename Provider::distance_type;

	distance_type best_distance{};
	bool has_best = false;
	for (const auto medoid : medoids) {
		const auto distance = provider.distance(id, medoid);
		if (!has_best || distance < best_distance) {
			best_distance = distance;
			has_best = true;
		}
	}
	return best_distance;
}

template <typename Provider>
auto initialize_medoids(const Provider &provider, std::size_t cluster_count) -> std::vector<RecordId>
{
	using distance_type = typename Provider::distance_type;

	std::vector<RecordId> medoids;
	medoids.reserve(cluster_count);

	RecordId first_medoid;
	distance_type best_total{};
	bool has_best = false;
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		const auto candidate = RecordId::from_index(index);
		const auto total = total_distance_to_all(provider, candidate);
		if (!has_best || total < best_total) {
			first_medoid = candidate;
			best_total = total;
			has_best = true;
		}
	}
	medoids.push_back(first_medoid);

	while (medoids.size() < cluster_count) {
		RecordId next_medoid;
		distance_type best_distance{};
		bool has_next = false;
		for (std::size_t index = 0; index < provider.record_count(); ++index) {
			const auto candidate = RecordId::from_index(index);
			if (contains_medoid(medoids, candidate)) {
				continue;
			}
			const auto distance = nearest_medoid_distance(provider, medoids, candidate);
			if (!has_next || distance > best_distance ||
				(distance == best_distance && candidate.index() < next_medoid.index())) {
				next_medoid = candidate;
				best_distance = distance;
				has_next = true;
			}
		}
		if (!has_next) {
			throw std::logic_error("failed to initialize k-medoids");
		}
		medoids.push_back(next_medoid);
	}

	std::sort(medoids.begin(), medoids.end());
	return medoids;
}

template <typename Provider>
auto assign_to_medoids(const Provider &provider, const std::vector<RecordId> &medoids)
	-> std::pair<std::vector<std::size_t>, std::vector<std::size_t>>
{
	using distance_type = typename Provider::distance_type;

	std::vector<std::size_t> assignments(provider.record_count(), 0);
	std::vector<std::size_t> cluster_sizes(medoids.size(), 0);

	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		const auto id = RecordId::from_index(index);
		std::size_t best_cluster = 0;
		distance_type best_distance{};
		bool has_best = false;
		for (std::size_t cluster = 0; cluster < medoids.size(); ++cluster) {
			const auto distance = provider.distance(id, medoids[cluster]);
			if (!has_best || distance < best_distance ||
				(distance == best_distance && medoids[cluster].index() < medoids[best_cluster].index())) {
				best_cluster = cluster;
				best_distance = distance;
				has_best = true;
			}
		}
		assignments[index] = best_cluster;
		++cluster_sizes[best_cluster];
	}

	return {assignments, cluster_sizes};
}

template <typename Provider>
auto recompute_medoids(const Provider &provider, const std::vector<std::size_t> &assignments,
					   const std::vector<std::size_t> &cluster_sizes, const std::vector<RecordId> &current_medoids)
	-> std::vector<RecordId>
{
	using distance_type = typename Provider::distance_type;

	std::vector<RecordId> medoids;
	medoids.reserve(current_medoids.size());

	for (std::size_t cluster = 0; cluster < current_medoids.size(); ++cluster) {
		if (cluster_sizes[cluster] == 0) {
			medoids.push_back(current_medoids[cluster]);
			continue;
		}

		RecordId best_medoid;
		distance_type best_total{};
		bool has_best = false;
		for (std::size_t candidate_index = 0; candidate_index < assignments.size(); ++candidate_index) {
			if (assignments[candidate_index] != cluster) {
				continue;
			}
			const auto candidate = RecordId::from_index(candidate_index);
			distance_type total{};
			for (std::size_t other_index = 0; other_index < assignments.size(); ++other_index) {
				if (assignments[other_index] == cluster) {
					total += provider.distance(candidate, RecordId::from_index(other_index));
				}
			}
			if (!has_best || total < best_total ||
				(total == best_total && candidate.index() < best_medoid.index())) {
				best_medoid = candidate;
				best_total = total;
				has_best = true;
			}
		}
		medoids.push_back(best_medoid);
	}

	std::sort(medoids.begin(), medoids.end());
	return medoids;
}

template <typename Provider>
auto compute_cluster_medoids(const Provider &provider, const std::vector<std::size_t> &assignments,
							 std::size_t cluster_count) -> std::vector<RecordId>
{
	using distance_type = typename Provider::distance_type;

	std::vector<RecordId> medoids;
	medoids.reserve(cluster_count);

	for (std::size_t cluster = 0; cluster < cluster_count; ++cluster) {
		RecordId best_medoid;
		distance_type best_total{};
		bool has_best = false;

		for (std::size_t candidate_index = 0; candidate_index < assignments.size(); ++candidate_index) {
			if (assignments[candidate_index] != cluster) {
				continue;
			}
			const auto candidate = RecordId::from_index(candidate_index);
			distance_type total{};
			for (std::size_t other_index = 0; other_index < assignments.size(); ++other_index) {
				if (assignments[other_index] == cluster) {
					total += provider.distance(candidate, RecordId::from_index(other_index));
				}
			}
			if (!has_best || total < best_total ||
				(total == best_total && candidate.index() < best_medoid.index())) {
				best_medoid = candidate;
				best_total = total;
				has_best = true;
			}
		}

		if (has_best) {
			medoids.push_back(best_medoid);
		}
	}

	return medoids;
}

template <typename Radius> auto validate_dbscan_parameters(Radius radius, std::size_t min_points) -> void
{
	if (radius < Radius{}) {
		throw std::invalid_argument("radius must be non-negative");
	}
	if (min_points == 0) {
		throw std::invalid_argument("min_points must be positive");
	}
}

template <typename Provider, typename Radius>
auto dbscan_region_query(const Provider &provider, RecordId id, Radius radius) -> std::vector<RecordId>
{
	std::vector<RecordId> neighbors;
	neighbors.reserve(provider.record_count());

	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		const auto candidate = RecordId::from_index(index);
		if (provider.distance(id, candidate) <= radius) {
			neighbors.push_back(candidate);
		}
	}

	return neighbors;
}

template <typename Provider, typename Radius>
auto expand_dbscan_cluster(const Provider &provider, const std::vector<RecordId> &seed_neighbors, Radius radius,
						   std::size_t min_points, std::size_t cluster, std::vector<bool> &visited,
						   std::vector<bool> &assigned, std::vector<std::size_t> &assignments,
						   std::vector<bool> &core_records) -> void
{
	std::vector<RecordId> queue;
	std::vector<bool> queued(provider.record_count(), false);
	queue.reserve(seed_neighbors.size());

	for (const auto neighbor : seed_neighbors) {
		queue.push_back(neighbor);
		queued[neighbor.index()] = true;
	}

	for (std::size_t cursor = 0; cursor < queue.size(); ++cursor) {
		const auto candidate = queue[cursor];
		const auto candidate_index = candidate.index();

		if (!visited[candidate_index]) {
			visited[candidate_index] = true;
			auto neighbors = dbscan_region_query(provider, candidate, radius);
			if (neighbors.size() >= min_points) {
				core_records[candidate_index] = true;
				for (const auto neighbor : neighbors) {
					if (!queued[neighbor.index()]) {
						queue.push_back(neighbor);
						queued[neighbor.index()] = true;
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

} // namespace engine_detail

template <typename Provider, typename std::enable_if<DistanceProvider_v<Provider>, int>::type = 0>
auto kmedoids(const Provider &provider, std::size_t cluster_count, std::size_t max_iterations = 100)
	-> ClusteringResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot cluster an empty distance provider");
	}
	if (cluster_count == 0) {
		throw std::invalid_argument("cluster_count must be positive");
	}
	if (cluster_count > provider.record_count()) {
		throw std::invalid_argument("cluster_count cannot exceed record_count");
	}

	auto medoids = engine_detail::initialize_medoids(provider, cluster_count);
	auto assignment_state = engine_detail::assign_to_medoids(provider, medoids);
	auto assignments = std::move(assignment_state.first);
	auto cluster_sizes = std::move(assignment_state.second);

	std::size_t iterations = 0;
	bool converged = false;
	for (; iterations < max_iterations; ++iterations) {
		auto updated_medoids = engine_detail::recompute_medoids(provider, assignments, cluster_sizes, medoids);
		auto updated_assignment_state = engine_detail::assign_to_medoids(provider, updated_medoids);
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

	ClusteringResult<distance_type> result;
	result.assignments = std::move(assignments);
	result.medoids = std::move(medoids);
	result.cluster_sizes = std::move(cluster_sizes);
	result.record_count = provider.record_count();
	result.cluster_count = cluster_count;
	result.iterations = iterations;
	result.converged = converged;
	result.algorithm = "kmedoids";
	result.representation = "distance_provider";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto kmedoids(const Space &space, std::size_t cluster_count, std::size_t max_iterations = 100)
	-> ClusteringResult<typename Space::distance_type>
{
	representations::ImplicitDistanceProvider<Space> provider(space);
	auto result = kmedoids(provider, cluster_count, max_iterations);
	result.representation = "metric_space";
	return result;
}

template <typename Provider, typename Radius, typename std::enable_if<DistanceProvider_v<Provider>, int>::type = 0>
auto dbscan(const Provider &provider, Radius radius, std::size_t min_points)
	-> ClusteringResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	const auto noise_label = ClusteringResult<distance_type>::noise_label;

	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot cluster an empty distance provider");
	}
	engine_detail::validate_dbscan_parameters(radius, min_points);

	std::vector<std::size_t> assignments(provider.record_count(), noise_label);
	std::vector<bool> assigned(provider.record_count(), false);
	std::vector<bool> visited(provider.record_count(), false);
	std::vector<bool> core_record_flags(provider.record_count(), false);

	std::size_t cluster_count = 0;
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		if (visited[index]) {
			continue;
		}

		const auto id = RecordId::from_index(index);
		visited[index] = true;
		auto neighbors = engine_detail::dbscan_region_query(provider, id, radius);
		if (neighbors.size() < min_points) {
			continue;
		}

		core_record_flags[index] = true;
		engine_detail::expand_dbscan_cluster(provider, neighbors, radius, min_points, cluster_count, visited, assigned,
											 assignments, core_record_flags);
		++cluster_count;
	}

	std::vector<std::size_t> cluster_sizes(cluster_count, 0);
	std::vector<RecordId> core_records;
	std::vector<RecordId> noise_records;
	std::size_t noise_count = 0;
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		if (assigned[index]) {
			++cluster_sizes[assignments[index]];
		} else {
			assignments[index] = noise_label;
			noise_records.push_back(RecordId::from_index(index));
			++noise_count;
		}

		if (core_record_flags[index]) {
			core_records.push_back(RecordId::from_index(index));
		}
	}

	ClusteringResult<distance_type> result;
	result.assignments = std::move(assignments);
	result.medoids = engine_detail::compute_cluster_medoids(provider, result.assignments, cluster_count);
	result.core_records = std::move(core_records);
	result.noise_records = std::move(noise_records);
	result.cluster_sizes = std::move(cluster_sizes);
	result.record_count = provider.record_count();
	result.cluster_count = cluster_count;
	result.noise_count = noise_count;
	result.iterations = 1;
	result.converged = true;
	result.algorithm = "dbscan";
	result.representation = "distance_provider";
	return result;
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto dbscan(const Space &space, Radius radius, std::size_t min_points) -> ClusteringResult<typename Space::distance_type>
{
	representations::ImplicitDistanceProvider<Space> provider(space);
	auto result = dbscan(provider, radius, min_points);
	result.representation = "metric_space";
	return result;
}

} // namespace metric::operators

#endif
