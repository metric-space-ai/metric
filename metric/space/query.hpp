// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_QUERY_HPP
#define _METRIC_SPACE_QUERY_HPP

#include <cstddef>
#include <optional>
#include <type_traits>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/neighbor.hpp>
#include <metric/core/result.hpp>
#include <metric/record/id.hpp>
#include <metric/stats/search/nearest.hpp>
#include <metric/stats/search/neighbors.hpp>
#include <metric/stats/search/options.hpp>

// Ergonomic, space-centric entry points for neighbour queries.
//
// mtrc::space::query is a thin facade for a user who is holding a finite metric space and just wants
// "the nearest record", "the k nearest", or "everything within a radius". It owns NO search algorithm:
// every call forwards to mtrc::stats::search (which is the conceptual home of search) and, for indexed
// variants, to the existing mtrc::space::storage indexes. The value it adds is discoverability and a
// couple of conveniences stats::search does not spell out directly, most importantly a single-nearest
// helper with an explicit empty/no-other-record contract.
//
// Both RecordId queries (a record already in the space; the query record is excluded from its own
// result) and external-record queries (a probe record that need not be in the space) are supported.
namespace mtrc::space::query {

// ---------------------------------------------------------------------------------------------
// k nearest neighbours (delegates to stats::search::knn / find_neighbors).
// ---------------------------------------------------------------------------------------------

// k nearest records to a record already in the space (excludes the query record itself).
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto k_nearest(const Space &space, RecordId query_id, std::size_t k)
	-> NeighborSet<typename Space::distance_type>
{
	return stats::search::knn(space, query_id, k);
}

// k nearest records to an external probe record.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto k_nearest(const Space &space, const typename Space::record_type &query, std::size_t k)
	-> NeighborSet<typename Space::distance_type>
{
	return stats::search::knn(space, query, k);
}

// k nearest records to an external probe record, using an explicit execution strategy from
// stats::search (e.g. stats::search::cover_tree{}, distance_table{}, knn_graph{}, exact_scan{}). This
// is how a caller picks a representation without constructing any low-level storage/index type by hand.
template <typename Space, typename Strategy,
		  typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto k_nearest(const Space &space, const typename Space::record_type &query, std::size_t k, Strategy strategy)
	-> NeighborSet<typename Space::distance_type>
{
	return stats::search::find_neighbors(space, query, k, strategy);
}

// k nearest records to a record already in the space, using an explicit execution strategy.
template <typename Space, typename Strategy,
		  typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto k_nearest(const Space &space, RecordId query_id, std::size_t k, Strategy strategy)
	-> NeighborSet<typename Space::distance_type>
{
	return stats::search::find_neighbors(space, query_id, k, strategy);
}

// ---------------------------------------------------------------------------------------------
// Single nearest neighbour.
// ---------------------------------------------------------------------------------------------

// Nearest other record to a record in the space, or empty if the space has no other record.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto try_nearest(const Space &space, RecordId query_id)
	-> std::optional<Neighbor<typename Space::distance_type>>
{
	auto result = stats::search::knn(space, query_id, 1);
	if (result.empty()) {
		return std::nullopt;
	}
	return result[0];
}

// Nearest record to an external probe record, or empty if the space is empty.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto try_nearest(const Space &space, const typename Space::record_type &query)
	-> std::optional<Neighbor<typename Space::distance_type>>
{
	auto result = stats::search::knn(space, query, 1);
	if (result.empty()) {
		return std::nullopt;
	}
	return result[0];
}

// Nearest other record to a record in the space. Throws MetricInputError if the space holds no other
// record (a single-record or empty space has no neighbour to return).
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto nearest(const Space &space, RecordId query_id) -> Neighbor<typename Space::distance_type>
{
	auto result = try_nearest(space, query_id);
	if (!result.has_value()) {
		throw MetricInputError("nearest(): the metric space has no other record than the query record");
	}
	return *result;
}

// Nearest record to an external probe record. Throws MetricInputError if the space is empty.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto nearest(const Space &space, const typename Space::record_type &query)
	-> Neighbor<typename Space::distance_type>
{
	auto result = try_nearest(space, query);
	if (!result.has_value()) {
		throw MetricInputError("nearest(): the metric space is empty");
	}
	return *result;
}

// ---------------------------------------------------------------------------------------------
// Radius / range neighbours (delegates to stats::search::range).
// ---------------------------------------------------------------------------------------------

// Every record within `radius` of a record in the space (excludes the query record itself). Throws
// std::invalid_argument for a negative radius.
template <typename Space, typename Radius,
		  typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto within(const Space &space, RecordId query_id, Radius radius)
	-> NeighborSet<typename Space::distance_type>
{
	return stats::search::range(space, query_id, radius);
}

// Every record within `radius` of an external probe record. Throws std::invalid_argument for a
// negative radius.
template <typename Space, typename Radius,
		  typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto within(const Space &space, const typename Space::record_type &query, Radius radius)
	-> NeighborSet<typename Space::distance_type>
{
	return stats::search::range(space, query, radius);
}

} // namespace mtrc::space::query

#endif
