// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_NEIGHBORS_HPP
#define _METRIC_INTENT_NEIGHBORS_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>

#include "../core/concepts.hpp"
#include "../core/parameters.hpp"
#include "../core/record_id.hpp"
#include "../core/result.hpp"
#include "../operators/nearest.hpp"
#include "../representations/cover_tree_index.hpp"
#include "../representations/knn_graph_index.hpp"
#include "../representations/matrix_cache.hpp"
#include "../runtime/execution.hpp"
#include "../strategies/search.hpp"

namespace metric::intent {

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
					strategies::brute_force = {}) -> NeighborSet<typename Space::distance_type>
{
	return operators::knn(space, query, count);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::metric::count requested,
					strategies::brute_force strategy = {}) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count, strategies::brute_force = {})
	-> NeighborSet<typename Space::distance_type>
{
	return operators::knn(space, query_id, count);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::metric::count requested,
					strategies::brute_force strategy = {}) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count, strategies::matrix_cache)
	-> NeighborSet<typename Space::distance_type>
{
	representations::MatrixCache<Space> matrix(space);
	auto result = operators::knn(matrix, query_id, count);
	result.representation = "matrix_cache";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::metric::count requested, strategies::matrix_cache strategy)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
					strategies::cover_tree) -> NeighborSet<typename Space::distance_type>
{
	representations::CoverTreeIndex<Space> index(space);
	auto result = operators::knn(index, query, count);
	result.representation = "cover_tree_index";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::metric::count requested,
					strategies::cover_tree strategy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
					strategies::knn_graph strategy) -> NeighborSet<typename Space::distance_type>
{
	const auto graph_neighbors = strategy.graph_neighbors == 0 ? count : strategy.graph_neighbors;
	representations::KnnGraphIndex<Space> index(space, graph_neighbors);
	auto result = operators::knn(index, query, count);
	result.representation = "knn_graph_index";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::metric::count requested,
					strategies::knn_graph strategy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count, strategies::knn_graph strategy)
	-> NeighborSet<typename Space::distance_type>
{
	if (query_id.index() >= space.size()) {
		throw std::out_of_range("query record id is outside the metric space");
	}
	const auto graph_neighbors = strategy.graph_neighbors == 0 ? count : strategy.graph_neighbors;
	representations::KnnGraphIndex<Space> index(space, graph_neighbors);

	NeighborSet<typename Space::distance_type> result;
	result.neighbors = index.neighbors(query_id);
	if (result.neighbors.size() > count) {
		result.neighbors.resize(count);
	}
	result.record_count = index.record_count();
	result.requested_count = count;
	result.exact = true;
	result.operator_name = "knn";
	result.representation = "knn_graph_index";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::metric::count requested, strategies::knn_graph strategy)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
					runtime::policy runtime_policy) -> NeighborSet<typename Space::distance_type>
{
	runtime::require_exact_neighbors(runtime_policy);
	runtime::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	switch (runtime_policy.representation_mode()) {
	case runtime::representation::matrix_cache:
		throw std::invalid_argument("materialized neighbor runtime policy requires a RecordId query");
	case runtime::representation::cover_tree: {
		representations::CoverTreeIndex<Space> index(space);
		auto result = operators::knn(index, query, count);
		result.representation = runtime::neighbor_representation(runtime_policy);
		return result;
	}
	case runtime::representation::knn_graph: {
		const auto graph_neighbors = runtime_policy.graph_neighbors() == 0 ? count : runtime_policy.graph_neighbors();
		representations::KnnGraphIndex<Space> index(space, graph_neighbors);
		auto result = operators::knn(index, query, count);
		result.representation = runtime::neighbor_representation(runtime_policy);
		return result;
	}
	case runtime::representation::implicit: {
		auto result = find_neighbors(space, query, count, strategies::brute_force{});
		result.representation = runtime::neighbor_representation(runtime_policy);
		return result;
	}
	case runtime::representation::automatic: {
		if (runtime_policy.uses_materialization()) {
			throw std::invalid_argument("materialized neighbor runtime policy requires a RecordId query");
		}
		auto result = find_neighbors(space, query, count, strategies::brute_force{});
		result.representation = runtime::neighbor_representation(runtime_policy);
		return result;
	}
	}
	return find_neighbors(space, query, count, strategies::brute_force{});
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::metric::count requested,
					runtime::policy runtime_policy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value, runtime_policy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count, runtime::policy runtime_policy)
	-> NeighborSet<typename Space::distance_type>
{
	runtime::require_exact_neighbors(runtime_policy);
	runtime::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.representation_mode() == runtime::representation::matrix_cache ||
		(runtime_policy.representation_mode() == runtime::representation::automatic &&
		 runtime_policy.uses_materialization())) {
		representations::MatrixCache<Space> matrix(space);
		auto result = operators::knn(matrix, query_id, count);
		result.representation = runtime::neighbor_representation(runtime_policy);
		return result;
	}

	if (runtime_policy.representation_mode() == runtime::representation::cover_tree) {
		representations::CoverTreeIndex<Space> index(space);
		NeighborSet<typename Space::distance_type> result;
		result.record_count = index.record_count();
		result.requested_count = count;
		result.exact = true;
		result.operator_name = "knn";
		result.representation = runtime::neighbor_representation(runtime_policy);
		if (count == 0) {
			return result;
		}

		auto neighbors = index.knn(space.record(query_id), count + 1);
		for (const auto &neighbor : neighbors) {
			if (neighbor.id == query_id) {
				continue;
			}
			result.neighbors.push_back(neighbor);
			if (result.neighbors.size() == count) {
				break;
			}
		}
		return result;
	}

	if (runtime_policy.representation_mode() == runtime::representation::knn_graph) {
		const auto graph_neighbors = runtime_policy.graph_neighbors() == 0 ? count : runtime_policy.graph_neighbors();
		auto result = find_neighbors(space, query_id, count, strategies::knn_graph(graph_neighbors));
		result.representation = runtime::neighbor_representation(runtime_policy);
		return result;
	}

	auto result = operators::knn(space, query_id, count);
	result.representation = runtime::neighbor_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::metric::count requested, runtime::policy runtime_policy)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value, runtime_policy);
}

} // namespace metric::intent

namespace metric {
using intent::find_neighbors;
} // namespace metric

#endif
