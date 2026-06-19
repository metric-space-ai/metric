// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_DIAGNOSTICS_HPP
#define _METRIC_REPRESENTATIONS_DIAGNOSTICS_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace metric::representations {

enum class representation_kind {
	implicit_distance_provider,
	matrix_cache,
	cover_tree_index,
	knn_graph_index,
	graph_topology,
};

enum class exactness {
	exact,
	approximate,
};

enum class materialization {
	lazy,
	materialized,
	topology,
};

enum class update_mode {
	live,
	snapshot,
};

struct representation_diagnostics {
	representation_kind kind;
	exactness exact;
	materialization materialized;
	update_mode updates;
	std::size_t space_version{};
	std::size_t built_for_version{};
	bool stale{};
	std::size_t records{};
	std::size_t distance_evaluations{};
	std::size_t cached_distances{};
	std::size_t memory_bytes_estimate{};
	std::vector<std::string> warnings;
};

struct matrix_cache_stats {
	std::size_t hits{};
	std::size_t misses{};
	double fill_ratio{};
	bool symmetric_storage{};
};

struct cover_tree_stats {
	std::size_t nodes{};
	bool covering_validated{};
	bool covering_valid{};
};

struct knn_graph_stats {
	std::size_t nodes{};
	std::size_t edges{};
	std::size_t neighbors_requested{};
	bool recall_validated{};
	double sampled_recall{};
};

} // namespace metric::representations

#endif
