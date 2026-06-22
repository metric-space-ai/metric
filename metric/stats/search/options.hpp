// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STRATEGIES_SEARCH_HPP
#define _METRIC_STRATEGIES_SEARCH_HPP

#include <cstddef>

namespace mtrc::stats::search {

struct exact_scan {};

struct brute_force {};

struct distance_table {};

struct cover_tree {};

struct knn_graph {
	knn_graph() = default;

	explicit knn_graph(std::size_t graph_neighbors) : graph_neighbors(graph_neighbors) {}

	std::size_t graph_neighbors{};
};

} // namespace mtrc::stats::search

#endif
