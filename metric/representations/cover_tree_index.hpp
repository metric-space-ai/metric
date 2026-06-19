// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_COVER_TREE_INDEX_HPP
#define _METRIC_REPRESENTATIONS_COVER_TREE_INDEX_HPP

#include <algorithm>
#include <cstddef>
#include <vector>

#include "../core/neighbor.hpp"
#include "../core/record_id.hpp"

namespace metric::representations {

template <typename Space> class CoverTreeIndex {
  public:
	using space_type = Space;
	using record_type = typename space_type::record_type;
	using distance_type = typename space_type::distance_type;
	using neighbor_type = Neighbor<distance_type>;

	explicit CoverTreeIndex(const space_type &space)
		: space_(&space)
		, version_(space.version())
	{
	}

	auto knn(const record_type &query, std::size_t k) const -> std::vector<neighbor_type>
	{
		std::vector<neighbor_type> candidates;
		candidates.reserve(space_->size());

		for (std::size_t index = 0; index < space_->size(); ++index) {
			const auto id = RecordId::from_index(index);
			candidates.push_back(neighbor_type{id, space_->metric()(query, space_->record(id))});
		}

		sort_neighbors(candidates);
		if (candidates.size() > k) {
			candidates.resize(k);
		}
		return candidates;
	}

	auto record_count() const -> std::size_t { return space_->size(); }
	auto version() const -> std::size_t { return version_; }
	auto is_stale() const -> bool { return space_->version() != version_; }

  private:
	static auto sort_neighbors(std::vector<neighbor_type> &neighbors) -> void
	{
		std::sort(neighbors.begin(), neighbors.end(), [](const neighbor_type &lhs, const neighbor_type &rhs) {
			if (lhs.distance == rhs.distance) {
				return lhs.id < rhs.id;
			}
			return lhs.distance < rhs.distance;
		});
	}

	const space_type *space_;
	std::size_t version_;
};

} // namespace metric::representations

#endif
