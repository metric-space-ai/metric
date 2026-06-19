// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_KNN_GRAPH_INDEX_HPP
#define _METRIC_REPRESENTATIONS_KNN_GRAPH_INDEX_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "../core/neighbor.hpp"
#include "../core/record_id.hpp"
#include "diagnostics.hpp"

namespace metric::representations {

template <typename Space> class KnnGraphIndex {
  public:
	using space_type = Space;
	using record_type = typename space_type::record_type;
	using distance_type = typename space_type::distance_type;
	using neighbor_type = Neighbor<distance_type>;

	KnnGraphIndex(const space_type &space, std::size_t k)
		: space_(&space)
		, k_(k)
		, record_count_(space.size())
		, version_(space.version())
	{
		ids_.reserve(record_count_);
		records_.reserve(record_count_);
		for (std::size_t index = 0; index < record_count_; ++index) {
			const auto id = space.id(index);
			ids_.push_back(id);
			records_.push_back(space.record(id));
		}

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
		std::vector<neighbor_type> candidates;
		candidates.reserve(record_count_);
		for (std::size_t index = 0; index < record_count_; ++index) {
			candidates.push_back(neighbor_type{ids_[index], space_->metric()(query, records_[index])});
		}
		sort_neighbors(candidates);
		if (candidates.size() > k) {
			candidates.resize(k);
		}
		return candidates;
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
		for (std::size_t position = 0; position < ids_.size(); ++position) {
			if (ids_[position] == id) {
				return position;
			}
		}
		throw std::out_of_range("record id is outside the kNN graph index");
	}
	auto contains(RecordId id) const -> bool
	{
		for (const auto current : ids_) {
			if (current == id) {
				return true;
			}
		}
		return false;
	}
	auto version() const -> std::size_t { return version_; }
	auto is_stale() const -> bool { return space_->version() != version_; }
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

	template <typename Provider> auto stats_against(const Provider &provider, std::size_t sample_count = 0) const
		-> knn_graph_stats
	{
		auto result = stats();
		result.recall_validated = true;
		result.sampled_recall = sampled_recall(provider, sample_count);
		return result;
	}

	template <typename Provider> auto sampled_recall(const Provider &provider, std::size_t sample_count = 0) const -> double
	{
		if (k_ == 0 || record_count_ <= 1) {
			return 1.0;
		}

		const auto samples = sample_count == 0 || sample_count > record_count_ ? record_count_ : sample_count;
		double total_recall = 0.0;
		for (std::size_t source = 0; source < samples; ++source) {
			const auto source_id = ids_[source];
			auto exact_neighbors = exact_provider_neighbors(provider, source_id);
			const auto &graph_neighbors = adjacency_[source];
			std::size_t hits = 0;
			for (const auto &graph_neighbor : graph_neighbors) {
				const auto found =
					std::find_if(exact_neighbors.begin(), exact_neighbors.end(), [&](const neighbor_type &exact) {
						return exact.id == graph_neighbor.id;
					});
				if (found != exact_neighbors.end()) {
					++hits;
				}
			}
			const auto denominator = exact_neighbors.empty() ? 1.0 : static_cast<double>(exact_neighbors.size());
			total_recall += static_cast<double>(hits) / denominator;
		}
		return total_recall / static_cast<double>(samples);
	}

	auto diagnostics() const -> representation_diagnostics
	{
		representation_diagnostics result{representation_kind::knn_graph_index,
										  exactness::approximate,
										  materialization::materialized,
										  update_mode::snapshot};
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
		if (result.stale) {
			result.warnings.push_back("kNN graph index was built for an older metric-space version");
		}
		return result;
	}

  private:
	template <typename Provider>
	auto exact_provider_neighbors(const Provider &provider, RecordId source_id) const -> std::vector<neighbor_type>
	{
		std::vector<neighbor_type> candidates;
		candidates.reserve(record_count_ > 0 ? record_count_ - 1 : 0);
		for (std::size_t target = 0; target < provider.record_count(); ++target) {
			const auto target_id = provider.id(target);
			if (target_id == source_id) {
				continue;
			}
			candidates.push_back(neighbor_type{target_id, provider.distance(source_id, target_id)});
		}
		sort_neighbors(candidates);
		if (candidates.size() > k_) {
			candidates.resize(k_);
		}
		return candidates;
	}

	auto build_neighbors(std::size_t source_position) const -> std::vector<neighbor_type>
	{
		std::vector<neighbor_type> candidates;
		candidates.reserve(record_count_ > 0 ? record_count_ - 1 : 0);

		for (std::size_t target = 0; target < record_count_; ++target) {
			if (target == source_position) {
				continue;
			}
			candidates.push_back(neighbor_type{ids_[target],
											   space_->metric()(records_[source_position], records_[target])});
		}

		sort_neighbors(candidates);
		if (candidates.size() > k_) {
			candidates.resize(k_);
		}
		return candidates;
	}

	static auto sort_neighbors(std::vector<neighbor_type> &neighbors) -> void
	{
		std::sort(neighbors.begin(), neighbors.end(), [](const neighbor_type &lhs, const neighbor_type &rhs) {
			if (lhs.distance == rhs.distance) {
				return lhs.id < rhs.id;
			}
			return lhs.distance < rhs.distance;
		});
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
	std::vector<RecordId> ids_;
	std::vector<record_type> records_;
	std::vector<std::vector<neighbor_type>> adjacency_;
};

} // namespace metric::representations

#endif
