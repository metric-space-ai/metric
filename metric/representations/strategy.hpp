// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_STRATEGY_HPP
#define _METRIC_REPRESENTATIONS_STRATEGY_HPP

#include <stdexcept>
#include <type_traits>

#include "../core/concepts.hpp"
#include "../strategies/search.hpp"
#include "cover_tree_index.hpp"
#include "implicit.hpp"
#include "knn_graph_index.hpp"
#include "matrix_cache.hpp"

namespace metric::representations {

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto make(const Space &space, strategies::brute_force) -> ImplicitDistanceProvider<Space>
{
	return implicit(space);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto make(const Space &space, strategies::matrix_cache) -> MatrixCache<Space>
{
	return matrix(space);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto make(const Space &space, strategies::cover_tree) -> CoverTreeIndex<Space>
{
	return cover_tree(space);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto make(const Space &space, strategies::knn_graph strategy) -> KnnGraphIndex<Space>
{
	if (strategy.graph_neighbors == 0) {
		throw std::invalid_argument("kNN graph representation strategy requires positive neighbor count");
	}

	return knn_graph(space, strategy.graph_neighbors);
}

} // namespace metric::representations

#endif
