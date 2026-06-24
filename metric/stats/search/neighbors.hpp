// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_SEARCH_NEIGHBORS_HPP
#define _METRIC_STATS_SEARCH_NEIGHBORS_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_space.hpp>
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

namespace detail {

template <typename Distance, typename Radius>
auto range_from_neighbors(std::vector<Neighbor<Distance>> neighbors, Radius radius, std::size_t record_count,
						  const char *representation) -> NeighborSet<Distance>
{
	std::vector<Neighbor<Distance>> within;
	within.reserve(neighbors.size());
	for (const auto &neighbor : neighbors) {
		if (neighbor.distance <= radius) {
			within.push_back(neighbor);
		}
	}
	return core::range_neighbor_set(std::move(within), record_count, representation);
}

template <typename Distance>
auto exclude_neighbor_id(std::vector<Neighbor<Distance>> neighbors, RecordId excluded) -> std::vector<Neighbor<Distance>>
{
	std::vector<Neighbor<Distance>> filtered;
	filtered.reserve(neighbors.size());
	for (const auto &neighbor : neighbors) {
		if (neighbor.id != excluded) {
			filtered.push_back(neighbor);
		}
	}
	return filtered;
}

} // namespace detail

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

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_neighbors(const Container &records, const Metric &metric, const Record &query, std::size_t count)
	-> NeighborSet<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = find_neighbors(space, query, count);
	result.representation = "records";
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_neighbors(const Container &records, const Metric &metric, const Record &query, ::mtrc::count requested)
	-> NeighborSet<metric_result_t<Metric, Record>>
{
	return find_neighbors(records, metric, query, requested.value);
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

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, const typename Space::record_type &query, Radius radius,
		   stats::search::exact_scan) -> NeighborSet<typename Space::distance_type>
{
	engine_detail::validate_radius(radius);
	space::storage::ExactScanIndex<Space> index(space);
	return detail::range_from_neighbors(index.knn(query, index.record_count()), radius, index.record_count(),
										"exact_scan_index");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, RecordId query_id, Radius radius, stats::search::exact_scan)
	-> NeighborSet<typename Space::distance_type>
{
	engine_detail::validate_radius(radius);
	(void)space.record(query_id);
	space::storage::ExactScanIndex<Space> index(space);
	auto neighbors = detail::exclude_neighbor_id(index.knn(space.record(query_id), index.record_count()), query_id);
	return detail::range_from_neighbors(std::move(neighbors), radius, index.record_count(), "exact_scan_index");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, const typename Space::record_type &query, Radius radius,
		   stats::search::brute_force) -> NeighborSet<typename Space::distance_type>
{
	return range(space, query, radius, stats::search::exact_scan{});
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, RecordId query_id, Radius radius, stats::search::brute_force)
	-> NeighborSet<typename Space::distance_type>
{
	return range(space, query_id, radius, stats::search::exact_scan{});
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &, const typename Space::record_type &, Radius, stats::search::distance_table)
	-> NeighborSet<typename Space::distance_type>
{
	throw InvalidRuntimePolicyError("distance-table range strategy requires a RecordId query");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, RecordId query_id, Radius radius, stats::search::distance_table)
	-> NeighborSet<typename Space::distance_type>
{
	space::storage::DistanceTable<Space> matrix(space);
	auto result = stats::search::range(matrix, query_id, radius);
	result.representation = "distance_table";
	return result;
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

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, const typename Space::record_type &query, Radius radius,
		   stats::search::cover_tree) -> NeighborSet<typename Space::distance_type>
{
	engine_detail::validate_radius(radius);
	space::storage::CoverTreeIndex<Space> index(space);
	return detail::range_from_neighbors(index.knn(query, index.record_count()), radius, index.record_count(),
										"cover_tree_index");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, RecordId query_id, Radius radius, stats::search::cover_tree)
	-> NeighborSet<typename Space::distance_type>
{
	engine_detail::validate_radius(radius);
	(void)space.record(query_id);
	space::storage::CoverTreeIndex<Space> index(space);
	auto neighbors = detail::exclude_neighbor_id(index.knn(space.record(query_id), index.record_count()), query_id);
	return detail::range_from_neighbors(std::move(neighbors), radius, index.record_count(), "cover_tree_index");
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

template <typename Container, typename Metric, typename Strategy,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_neighbors(const Container &records, const Metric &metric, const Record &query, std::size_t count,
					Strategy strategy) -> NeighborSet<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = find_neighbors(space, query, count, strategy);
	return result;
}

template <typename Container, typename Metric, typename Strategy,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_neighbors(const Container &records, const Metric &metric, const Record &query, ::mtrc::count requested,
					Strategy strategy) -> NeighborSet<metric_result_t<Metric, Record>>
{
	return find_neighbors(records, metric, query, requested.value, strategy);
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, const typename Space::record_type &query, Radius radius,
		   stats::search::knn_graph strategy) -> NeighborSet<typename Space::distance_type>
{
	engine_detail::validate_radius(radius);
	const auto graph_neighbors = strategy.graph_neighbors == 0 ? space.size() : strategy.graph_neighbors;
	space::storage::KnnGraphIndex<Space> index(space, graph_neighbors);
	return detail::range_from_neighbors(index.knn(query, index.record_count()), radius, index.record_count(),
										"knn_graph_index");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, RecordId query_id, Radius radius, stats::search::knn_graph strategy)
	-> NeighborSet<typename Space::distance_type>
{
	engine_detail::validate_radius(radius);
	(void)space.record(query_id);
	const auto available_neighbors = space.size() > 0 ? space.size() - 1 : 0;
	const auto graph_neighbors = strategy.graph_neighbors == 0 ? available_neighbors : strategy.graph_neighbors;
	if (graph_neighbors < available_neighbors) {
		throw RepresentationError("kNN graph range lookup requires graph_neighbors >= space.size() - 1");
	}
	space::storage::KnnGraphIndex<Space> index(space, graph_neighbors);
	return detail::range_from_neighbors(index.neighbors(query_id), radius, index.record_count(), "knn_graph_index");
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

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, const typename Space::record_type &query, Radius radius,
		   space::storage::policy runtime_policy) -> NeighborSet<typename Space::distance_type>
{
	space::storage::require_exact_neighbors(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	switch (runtime_policy.representation_mode()) {
	case space::storage::representation::distance_table:
		throw InvalidRuntimePolicyError("materialized range runtime policy requires a RecordId query");
	case space::storage::representation::cover_tree: {
		auto result = range(space, query, radius, stats::search::cover_tree{});
		result.representation = space::storage::neighbor_representation(runtime_policy);
		return result;
	}
	case space::storage::representation::knn_graph: {
		auto result = range(space, query, radius, stats::search::knn_graph(runtime_policy.graph_neighbors()));
		result.representation = space::storage::neighbor_representation(runtime_policy);
		return result;
	}
	case space::storage::representation::implicit: {
		auto result = stats::search::range(space, query, radius);
		result.representation = space::storage::neighbor_representation(runtime_policy);
		return result;
	}
	case space::storage::representation::automatic: {
		if (runtime_policy.uses_materialization()) {
			throw InvalidRuntimePolicyError("materialized range runtime policy requires a RecordId query");
		}
		auto result = stats::search::range(space, query, radius);
		result.representation = space::storage::neighbor_representation(runtime_policy);
		return result;
	}
	}
	return stats::search::range(space, query, radius);
}

template <typename Container, typename Metric, typename Radius,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto range(const Container &records, const Metric &metric, const Record &query, Radius radius)
	-> NeighborSet<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = stats::search::range(space, query, radius);
	result.representation = "records";
	return result;
}

template <typename Container, typename Metric, typename Radius, typename Strategy,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto range(const Container &records, const Metric &metric, const Record &query, Radius radius, Strategy strategy)
	-> NeighborSet<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = range(space, query, radius, strategy);
	return result;
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, RecordId query_id, Radius radius, space::storage::policy runtime_policy)
	-> NeighborSet<typename Space::distance_type>
{
	space::storage::require_exact_neighbors(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	(void)space.record(query_id);
	if (runtime_policy.representation_mode() == space::storage::representation::distance_table ||
		(runtime_policy.representation_mode() == space::storage::representation::automatic &&
		 runtime_policy.uses_materialization())) {
		space::storage::DistanceTable<Space> matrix(space);
		auto result = stats::search::range(matrix, query_id, radius);
		result.representation = space::storage::neighbor_representation(runtime_policy);
		return result;
	}

	if (runtime_policy.representation_mode() == space::storage::representation::cover_tree) {
		auto result = range(space, query_id, radius, stats::search::cover_tree{});
		result.representation = space::storage::neighbor_representation(runtime_policy);
		return result;
	}

	if (runtime_policy.representation_mode() == space::storage::representation::knn_graph) {
		auto result = range(space, query_id, radius, stats::search::knn_graph(runtime_policy.graph_neighbors()));
		result.representation = space::storage::neighbor_representation(runtime_policy);
		return result;
	}

	auto result = stats::search::range(space, query_id, radius);
	result.representation = space::storage::neighbor_representation(runtime_policy);
	return result;
}

} // namespace mtrc::stats::search

namespace mtrc {
using stats::search::find_neighbors;
} // namespace mtrc

#endif
