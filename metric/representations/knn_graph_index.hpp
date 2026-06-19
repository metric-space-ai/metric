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
		adjacency_.reserve(record_count_);
		for (std::size_t source = 0; source < record_count_; ++source) {
			adjacency_.push_back(build_neighbors(RecordId::from_index(source)));
		}
	}

	auto neighbors(RecordId source) const -> const std::vector<neighbor_type> &
	{
		validate(source);
		return adjacency_[source.index()];
	}

	auto knn(const record_type &query, std::size_t k) const -> std::vector<neighbor_type>
	{
		std::vector<neighbor_type> candidates;
		candidates.reserve(record_count_);
		for (std::size_t index = 0; index < record_count_; ++index) {
			const auto id = RecordId::from_index(index);
			candidates.push_back(neighbor_type{id, space_->metric()(query, space_->record(id))});
		}
		sort_neighbors(candidates);
		if (candidates.size() > k) {
			candidates.resize(k);
		}
		return candidates;
	}

	auto k() const -> std::size_t { return k_; }
	auto record_count() const -> std::size_t { return record_count_; }
	auto version() const -> std::size_t { return version_; }
	auto is_stale() const -> bool { return space_->version() != version_; }

  private:
	auto build_neighbors(RecordId source) const -> std::vector<neighbor_type>
	{
		std::vector<neighbor_type> candidates;
		candidates.reserve(record_count_ > 0 ? record_count_ - 1 : 0);

		for (std::size_t target = 0; target < record_count_; ++target) {
			const auto target_id = RecordId::from_index(target);
			if (target_id == source) {
				continue;
			}
			candidates.push_back(neighbor_type{target_id, space_->distance(source, target_id)});
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

	auto validate(RecordId id) const -> void
	{
		if (id.index() >= record_count_) {
			throw std::out_of_range("record id is outside the kNN graph index");
		}
	}

	const space_type *space_;
	std::size_t k_;
	std::size_t record_count_;
	std::size_t version_;
	std::vector<std::vector<neighbor_type>> adjacency_;
};

} // namespace metric::representations

#endif
