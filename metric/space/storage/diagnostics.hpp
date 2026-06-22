// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_DIAGNOSTICS_HPP
#define _METRIC_REPRESENTATIONS_DIAGNOSTICS_HPP

#include <cstddef>
#include <initializer_list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <metric/record/id.hpp>

namespace mtrc::space::storage {

enum class representation_kind {
	live_distances,
	distance_table,
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
	std::size_t dense_distance_slots{};
	std::size_t max_dense_records{};
	std::size_t memory_bytes_estimate{};
	std::string cache_key;
	std::string metric_key;
	std::vector<RecordId> source_record_ids;
	std::vector<std::string> warnings;
};

struct distance_table_stats {
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

struct knn_graph_quality_diagnostics {
	std::string diagnostic{"knn_graph_quality"};
	std::string representation{"knn_graph_index"};
	std::string reference_representation{"distance_table"};
	std::size_t record_count{};
	std::size_t edge_count{};
	std::size_t requested_neighbors{};
	std::size_t evaluated_queries{};
	std::size_t evaluated_neighbor_count{};
	std::size_t matched_neighbors{};
	std::size_t possible_neighbors{};
	std::size_t exact_distance_evaluations{};
	std::size_t graph_edge_evaluations{};
	double recall{};
	double average_row_recall{};
	double minimum_row_recall{};
	double maximum_row_recall{};
	double average_best_distance_inflation{1.0};
	double maximum_best_distance_inflation{1.0};
	bool exact_reference{true};
	bool approximate_candidate{true};
	bool stale{};
	std::vector<std::string> warnings;
};

inline auto representation_cache_key(
	const std::string &representation, const std::string &metric_key, std::size_t version,
	const std::vector<RecordId> &ids,
	std::initializer_list<std::pair<std::string, std::string>> parameters = {}) -> std::string
{
	std::ostringstream output;
	output << representation << "|metric=" << metric_key << "|version=" << version;
	for (const auto &parameter : parameters) {
		output << "|" << parameter.first << "=" << parameter.second;
	}
	output << "|ids=";
	for (std::size_t index = 0; index < ids.size(); ++index) {
		if (index > 0) {
			output << ",";
		}
		output << ids[index].index();
	}
	return output.str();
}

} // namespace mtrc::space::storage

#endif
