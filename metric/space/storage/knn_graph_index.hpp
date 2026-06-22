// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_KNN_GRAPH_INDEX_HPP
#define _METRIC_REPRESENTATIONS_KNN_GRAPH_INDEX_HPP

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_traits.hpp>
#include <metric/core/neighbor.hpp>
#include <metric/record/id.hpp>
#include "diagnostics.hpp"

namespace mtrc::space::storage {

template <typename Space> class KnnGraphIndex {
  public:
	using space_type = Space;
	using record_type = typename space_type::record_type;
	using distance_type = typename space_type::distance_type;
	using neighbor_type = Neighbor<distance_type>;

	KnnGraphIndex(const space_type &space, std::size_t k)
		: space_(&space), k_(k), record_count_(space.size()), version_(space.version()),
		  metric_key_(core::metric_cache_key(space.metric()))
	{
		ids_ = mtrc::record_ids(space);
		records_ = mtrc::records_for_record_ids(space, ids_);
		cache_key_ = representation_cache_key("knn_graph_index", metric_key_, version_, ids_,
											  {{"k", std::to_string(k_)}});

		adjacency_.reserve(record_count_);
		for (std::size_t source = 0; source < record_count_; ++source) {
			adjacency_.push_back(build_neighbors(source));
		}
	}

	auto neighbors(RecordId source) const -> const std::vector<neighbor_type> &
	{
		return adjacency_[position_of(source)];
	}

	auto knn(const record_type &query, std::size_t k) const -> std::vector<neighbor_type>
	{
		auto candidates = core::neighbor_candidates<distance_type>(
			record_count_, [this](std::size_t index) { return ids_[index]; },
			[this, &query](RecordId, std::size_t index) { return space_->metric()(query, records_[index]); });
		return core::take_nearest_neighbors(std::move(candidates), k);
	}

	auto k() const -> std::size_t { return k_; }
	auto record_count() const -> std::size_t { return record_count_; }
	auto id(std::size_t position) const -> RecordId
	{
		validate_position(position);
		return ids_[position];
	}
	auto position_of(RecordId id) const -> std::size_t
	{
		return mtrc::position_of_record_id(ids_, id, "record id is outside the kNN graph index");
	}
	auto contains(RecordId id) const -> bool
	{
		return mtrc::contains_record_id(ids_, id);
	}
	auto version() const -> std::size_t { return version_; }
	auto is_stale() const -> bool { return space_->version() != version_; }
	auto metric_key() const -> const std::string & { return metric_key_; }
	auto cache_key() const -> const std::string & { return cache_key_; }
	auto source_record_ids() const -> const std::vector<RecordId> & { return ids_; }
	auto edge_count() const -> std::size_t
	{
		std::size_t count = 0;
		for (const auto &neighbors : adjacency_) {
			count += neighbors.size();
		}
		return count;
	}
	auto stats() const -> knn_graph_stats
	{
		knn_graph_stats result;
		result.nodes = record_count_;
		result.edges = edge_count();
		result.neighbors_requested = k_;
		return result;
	}

	template <typename Provider>
	auto stats_against(const Provider &provider, std::size_t sample_count = 0) const -> knn_graph_stats
	{
		auto result = stats();
		result.recall_validated = true;
		result.sampled_recall = quality_against(provider, sample_count).recall;
		return result;
	}

	template <typename Provider>
	auto sampled_recall(const Provider &provider, std::size_t sample_count = 0) const -> double
	{
		return quality_against(provider, sample_count).recall;
	}

	template <typename Provider>
	auto quality_against(const Provider &provider, std::size_t sample_count = 0) const -> knn_graph_quality_diagnostics
	{
		if (k_ == 0 || record_count_ <= 1) {
			knn_graph_quality_diagnostics result;
			result.record_count = record_count_;
			result.edge_count = edge_count();
			result.requested_neighbors = k_;
			result.evaluated_queries = record_count_;
			result.recall = 1.0;
			result.average_row_recall = 1.0;
			result.minimum_row_recall = 1.0;
			result.maximum_row_recall = 1.0;
			result.stale = is_stale();
			if (result.stale) {
				result.warnings.push_back("kNN graph quality was evaluated on a stale graph index");
			}
			return result;
		}

		const auto samples = sample_count == 0 || sample_count > record_count_ ? record_count_ : sample_count;
		knn_graph_quality_diagnostics result;
		result.record_count = record_count_;
		result.edge_count = edge_count();
		result.requested_neighbors = k_;
		result.evaluated_queries = samples;
		result.evaluated_neighbor_count = k_;
		result.exact_distance_evaluations = samples * (provider.record_count() > 0 ? provider.record_count() - 1 : 0);
		result.stale = is_stale();

		core::RecallAccumulator recall;
		core::ScalarAccumulator<double> best_distance_inflation_summary;
		for (std::size_t source = 0; source < samples; ++source) {
			const auto source_id = ids_[source];
			auto exact_neighbors = exact_provider_neighbors(provider, source_id);
			const auto &graph_neighbors = adjacency_[source];
			result.graph_edge_evaluations += graph_neighbors.size();
			const auto hits = core::neighbor_id_overlap_count(exact_neighbors, graph_neighbors);
			const auto possible = exact_neighbors.size();
			(void)recall.add(hits, possible, 1.0);

			double best_distance_inflation = 1.0;
			if (!exact_neighbors.empty() && !graph_neighbors.empty()) {
				const auto exact_best = static_cast<double>(exact_neighbors.front().distance);
				const auto graph_best = static_cast<double>(graph_neighbors.front().distance);
				if (exact_best == 0.0) {
					best_distance_inflation = graph_best == 0.0 ? 1.0 : std::numeric_limits<double>::infinity();
				} else {
					best_distance_inflation = graph_best / exact_best;
				}
			}
			best_distance_inflation_summary.add(best_distance_inflation);
		}
		result.matched_neighbors = recall.matched;
		result.possible_neighbors = recall.possible;
		result.recall = recall.total_recall(1.0);
		result.average_row_recall = recall.average_recall(1.0);
		result.minimum_row_recall = recall.minimum_recall_or(1.0);
		result.maximum_row_recall = recall.maximum_recall_or(1.0);
		result.average_best_distance_inflation = best_distance_inflation_summary.average_or(1.0);
		result.maximum_best_distance_inflation = best_distance_inflation_summary.maximum_or(1.0);
		if (result.stale) {
			result.warnings.push_back("kNN graph quality was evaluated on a stale graph index");
		}
		return result;
	}

	auto diagnostics() const -> representation_diagnostics
	{
		representation_diagnostics result{representation_kind::knn_graph_index, exactness::approximate,
										  materialization::materialized, update_mode::snapshot};
		result.space_version = space_->version();
		result.built_for_version = version_;
		result.stale = is_stale();
		result.records = record_count_;
		result.distance_evaluations = record_count_ * (record_count_ > 0 ? record_count_ - 1 : 0);
		result.cached_distances = edge_count();
		result.memory_bytes_estimate = ids_.size() * sizeof(RecordId) + records_.size() * sizeof(record_type);
		for (const auto &neighbors : adjacency_) {
			result.memory_bytes_estimate += neighbors.size() * sizeof(neighbor_type);
		}
		result.cache_key = cache_key_;
		result.metric_key = metric_key_;
		result.source_record_ids = ids_;
		if (result.stale) {
			result.warnings.push_back("kNN graph index was built for an older metric-space version");
		}
		return result;
	}

  private:
	template <typename Provider>
	auto exact_provider_neighbors(const Provider &provider, RecordId source_id) const -> std::vector<neighbor_type>
	{
		auto candidates = core::neighbor_candidates_if<distance_type>(
			provider.record_count(), [&provider](std::size_t target) { return provider.id(target); },
			[&provider, source_id](RecordId target_id, std::size_t) { return provider.distance(source_id, target_id); },
			[source_id](RecordId target_id, std::size_t) { return target_id != source_id; });
		return core::take_nearest_neighbors(std::move(candidates), k_);
	}

	auto build_neighbors(std::size_t source_position) const -> std::vector<neighbor_type>
	{
		auto candidates = core::neighbor_candidates_if<distance_type>(
			record_count_, [this](std::size_t target) { return ids_[target]; },
			[this, source_position](RecordId, std::size_t target) {
				return space_->metric()(records_[source_position], records_[target]);
			},
			[source_position](RecordId, std::size_t target) { return target != source_position; });
		return core::take_nearest_neighbors(std::move(candidates), k_);
	}

	auto validate_position(std::size_t position) const -> void
	{
		if (position >= record_count_) {
			throw std::out_of_range("record position is outside the kNN graph index");
		}
	}

	const space_type *space_;
	std::size_t k_;
	std::size_t record_count_;
	std::size_t version_;
	std::string metric_key_;
	std::string cache_key_;
	std::vector<RecordId> ids_;
	std::vector<record_type> records_;
	std::vector<std::vector<neighbor_type>> adjacency_;
};

template <typename Space> auto knn_graph(const Space &space, std::size_t k) -> KnnGraphIndex<Space>
{
	return KnnGraphIndex<Space>(space, k);
}

template <typename Space> auto graph(const Space &space, std::size_t k) -> KnnGraphIndex<Space>
{
	return knn_graph(space, k);
}

} // namespace mtrc::space::storage

#endif
