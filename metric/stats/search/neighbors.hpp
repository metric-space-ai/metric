// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_NEIGHBORS_HPP
#define _METRIC_INTENT_NEIGHBORS_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/neighbor.hpp>
#include <metric/core/parameters.hpp>
#include <metric/record/id.hpp>
#include <metric/core/result.hpp>
#include <metric/stats/search/nearest.hpp>
#include <metric/space/storage/cover_tree_index.hpp>
#include <metric/space/storage/knn_graph_index.hpp>
#include <metric/space/storage/exact_scan_index.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/stats/search/options.hpp>

namespace mtrc::stats::search {

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count)
	-> NeighborSet<typename Space::distance_type>
{
	return stats::search::knn(space, query, count);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::mtrc::count requested)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count)
	-> NeighborSet<typename Space::distance_type>
{
	return stats::search::knn(space, query_id, count);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::mtrc::count requested)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
					stats::search::exact_scan) -> NeighborSet<typename Space::distance_type>
{
	space::storage::ExactScanIndex<Space> index(space);
	auto result = stats::search::knn(index, query, count);
	result.representation = "exact_scan_index";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::mtrc::count requested,
					stats::search::exact_scan strategy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count, stats::search::exact_scan)
	-> NeighborSet<typename Space::distance_type>
{
	space::storage::ExactScanIndex<Space> index(space);
	(void)space.record(query_id);
	auto neighbors = index.knn(space.record(query_id), count + 1);
	return core::make_neighbor_set(core::take_neighbors_excluding_id(neighbors, query_id, count),
								   index.record_count(), count, "knn", "exact_scan_index");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::mtrc::count requested,
					stats::search::exact_scan strategy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
					stats::search::brute_force) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, count, stats::search::exact_scan{});
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::mtrc::count requested,
					stats::search::brute_force strategy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count, stats::search::brute_force)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, count, stats::search::exact_scan{});
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::mtrc::count requested,
					stats::search::brute_force strategy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count, stats::search::distance_table)
	-> NeighborSet<typename Space::distance_type>
{
	space::storage::DistanceTable<Space> matrix(space);
	auto result = stats::search::knn(matrix, query_id, count);
	result.representation = "distance_table";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::mtrc::count requested, stats::search::distance_table strategy)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
					stats::search::cover_tree) -> NeighborSet<typename Space::distance_type>
{
	space::storage::CoverTreeIndex<Space> index(space);
	auto result = stats::search::knn(index, query, count);
	result.representation = "cover_tree_index";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::mtrc::count requested,
					stats::search::cover_tree strategy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
					stats::search::knn_graph strategy) -> NeighborSet<typename Space::distance_type>
{
	const auto graph_neighbors = strategy.graph_neighbors == 0 ? count : strategy.graph_neighbors;
	space::storage::KnnGraphIndex<Space> index(space, graph_neighbors);
	auto result = stats::search::knn(index, query, count);
	result.representation = "knn_graph_index";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::mtrc::count requested,
					stats::search::knn_graph strategy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count, stats::search::knn_graph strategy)
	-> NeighborSet<typename Space::distance_type>
{
	if (query_id.index() >= space.size()) {
		throw std::out_of_range("query record id is outside the metric space");
	}
	const auto graph_neighbors = strategy.graph_neighbors == 0 ? count : strategy.graph_neighbors;
	const auto available_neighbors = space.size() > 0 ? space.size() - 1 : 0;
	const auto required_graph_neighbors = count < available_neighbors ? count : available_neighbors;
	if (graph_neighbors < required_graph_neighbors) {
		throw RepresentationError("kNN graph RecordId lookup requires graph_neighbors >= requested neighbor count");
	}
	space::storage::KnnGraphIndex<Space> index(space, graph_neighbors);

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
auto find_neighbors(const Space &space, RecordId query_id, ::mtrc::count requested, stats::search::knn_graph strategy)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
					space::storage::policy runtime_policy) -> NeighborSet<typename Space::distance_type>
{
	space::storage::require_exact_neighbors(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	switch (runtime_policy.representation_mode()) {
	case space::storage::representation::distance_table:
		throw InvalidRuntimePolicyError("materialized neighbor runtime policy requires a RecordId query");
	case space::storage::representation::cover_tree: {
		space::storage::CoverTreeIndex<Space> index(space);
		auto result = stats::search::knn(index, query, count);
		result.representation = space::storage::neighbor_representation(runtime_policy);
		return result;
	}
	case space::storage::representation::knn_graph: {
		const auto graph_neighbors = runtime_policy.graph_neighbors() == 0 ? count : runtime_policy.graph_neighbors();
		space::storage::KnnGraphIndex<Space> index(space, graph_neighbors);
		auto result = stats::search::knn(index, query, count);
		result.representation = space::storage::neighbor_representation(runtime_policy);
		return result;
	}
	case space::storage::representation::implicit: {
		auto result = find_neighbors(space, query, count);
		result.representation = space::storage::neighbor_representation(runtime_policy);
		return result;
	}
	case space::storage::representation::automatic: {
		if (runtime_policy.uses_materialization()) {
			throw InvalidRuntimePolicyError("materialized neighbor runtime policy requires a RecordId query");
		}
		auto result = find_neighbors(space, query, count);
		result.representation = space::storage::neighbor_representation(runtime_policy);
		return result;
	}
	}
	return find_neighbors(space, query, count);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::mtrc::count requested,
					space::storage::policy runtime_policy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value, runtime_policy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count, space::storage::policy runtime_policy)
	-> NeighborSet<typename Space::distance_type>
{
	space::storage::require_exact_neighbors(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	(void)space.record(query_id);
	if (runtime_policy.representation_mode() == space::storage::representation::distance_table ||
		(runtime_policy.representation_mode() == space::storage::representation::automatic &&
		 runtime_policy.uses_materialization())) {
		space::storage::DistanceTable<Space> matrix(space);
		auto result = stats::search::knn(matrix, query_id, count);
		result.representation = space::storage::neighbor_representation(runtime_policy);
		return result;
	}

	if (runtime_policy.representation_mode() == space::storage::representation::cover_tree) {
		space::storage::CoverTreeIndex<Space> index(space);
		NeighborSet<typename Space::distance_type> result;
		result.record_count = index.record_count();
		result.requested_count = count;
		result.exact = true;
		result.operator_name = "knn";
		result.representation = space::storage::neighbor_representation(runtime_policy);
		if (count == 0) {
			return result;
		}

		const auto neighbors = index.knn(space.record(query_id), count + 1);
		result.neighbors = core::take_neighbors_excluding_id(neighbors, query_id, count);
		return result;
	}

	if (runtime_policy.representation_mode() == space::storage::representation::knn_graph) {
		const auto graph_neighbors = runtime_policy.graph_neighbors() == 0 ? count : runtime_policy.graph_neighbors();
		auto result = find_neighbors(space, query_id, count, stats::search::knn_graph(graph_neighbors));
		result.representation = space::storage::neighbor_representation(runtime_policy);
		return result;
	}

	auto result = stats::search::knn(space, query_id, count);
	result.representation = space::storage::neighbor_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::mtrc::count requested, space::storage::policy runtime_policy)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value, runtime_policy);
}

} // namespace mtrc::stats::search

namespace mtrc {
using stats::search::find_neighbors;
} // namespace mtrc

#endif
