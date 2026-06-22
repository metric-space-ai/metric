// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_STRATEGY_HPP
#define _METRIC_REPRESENTATIONS_STRATEGY_HPP

#include <stdexcept>
#include <type_traits>

#include <metric/core/concepts.hpp>
#include <metric/stats/search/options.hpp>
#include "cover_tree_index.hpp"
#include "exact_scan_index.hpp"
#include "implicit.hpp"
#include "knn_graph_index.hpp"
#include "distance_table.hpp"

namespace mtrc::space::storage {

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto make(const Space &space, stats::search::brute_force) -> LiveDistances<Space>
{
	return implicit(space);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto make(const Space &space, stats::search::exact_scan) -> ExactScanIndex<Space>
{
	return exact_scan(space);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto make(const Space &space, stats::search::distance_table) -> DistanceTable<Space>
{
	return matrix(space);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto make(const Space &space, stats::search::cover_tree) -> CoverTreeIndex<Space>
{
	return cover_tree(space);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto make(const Space &space, stats::search::knn_graph strategy) -> KnnGraphIndex<Space>
{
	if (strategy.graph_neighbors == 0) {
		throw std::invalid_argument("kNN graph representation strategy requires positive neighbor count");
	}

	return knn_graph(space, strategy.graph_neighbors);
}

} // namespace mtrc::space::storage

#endif
