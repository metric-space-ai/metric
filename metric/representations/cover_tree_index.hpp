// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_COVER_TREE_INDEX_HPP
#define _METRIC_REPRESENTATIONS_COVER_TREE_INDEX_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "../core/neighbor.hpp"
#include "../core/record_id.hpp"
#include "diagnostics.hpp"

namespace metric::representations {

template <typename Space> class CoverTreeIndex {
  public:
	using space_type = Space;
	using record_type = typename space_type::record_type;
	using distance_type = typename space_type::distance_type;
	using neighbor_type = Neighbor<distance_type>;

	explicit CoverTreeIndex(const space_type &space)
		: space_(&space)
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
		throw std::out_of_range("record id is outside the cover tree index");
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
	auto stats() const -> cover_tree_stats
	{
		cover_tree_stats result;
		result.nodes = record_count_;
		return result;
	}

	auto diagnostics() const -> representation_diagnostics
	{
		representation_diagnostics result{representation_kind::cover_tree_index,
										  exactness::exact,
										  materialization::materialized,
										  update_mode::snapshot};
		result.space_version = space_->version();
		result.built_for_version = version_;
		result.stale = is_stale();
		result.records = record_count_;
		result.memory_bytes_estimate = ids_.size() * sizeof(RecordId) + records_.size() * sizeof(record_type);
		if (result.stale) {
			result.warnings.push_back("cover tree index was built for an older metric-space version");
		}
		return result;
	}

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

	auto validate_position(std::size_t position) const -> void
	{
		if (position >= record_count_) {
			throw std::out_of_range("record position is outside the cover tree index");
		}
	}

	const space_type *space_;
	std::size_t record_count_;
	std::size_t version_;
	std::vector<RecordId> ids_;
	std::vector<record_type> records_;
};

template <typename Space> auto cover_tree(const Space &space) -> CoverTreeIndex<Space>
{
	return CoverTreeIndex<Space>(space);
}

} // namespace metric::representations

#endif
