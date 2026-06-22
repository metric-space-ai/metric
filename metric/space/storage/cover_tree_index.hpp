// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_COVER_TREE_INDEX_HPP
#define _METRIC_REPRESENTATIONS_COVER_TREE_INDEX_HPP

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_traits.hpp>
#include <metric/core/neighbor.hpp>
#include <metric/record/id.hpp>
#include <metric/space/tree.hpp>
#include "diagnostics.hpp"

namespace mtrc::space::storage {

template <typename Space> class CoverTreeIndex {
  public:
	using space_type = Space;
	using record_type = typename space_type::record_type;
	using metric_type = typename space_type::metric_type;
	using distance_type = typename space_type::distance_type;
	using neighbor_type = Neighbor<distance_type>;
	using tree_type = Tree<record_type, metric_type>;

	explicit CoverTreeIndex(const space_type &space)
		: space_(&space), record_count_(space.size()), version_(space.version()),
		  metric_key_(core::metric_cache_key(space.metric()))
	{
		require_true_metric_law();
		ids_ = mtrc::record_ids(space);
		records_ = mtrc::records_for_record_ids(space, ids_);
		cache_key_ = representation_cache_key("cover_tree_index", metric_key_, version_, ids_);
		if (records_.empty()) {
			tree_ = std::make_unique<tree_type>(-1, space.metric());
		} else {
			tree_ = std::make_unique<tree_type>(records_, -1, space.metric());
		}
	}

	auto knn(const record_type &query, std::size_t k) const -> std::vector<neighbor_type>
	{
		if (record_count_ == 0 || k == 0) {
			return {};
		}

		const auto requested = k > record_count_ ? record_count_ : k;
		const auto tree_neighbors = tree_->knn(query, static_cast<unsigned>(requested));
		std::vector<neighbor_type> neighbors;
		neighbors.reserve(tree_neighbors.size());
		for (const auto &neighbor : tree_neighbors) {
			if (neighbor.first == nullptr) {
				continue;
			}
			const auto local_id = static_cast<std::size_t>(neighbor.first->get_ID());
			if (local_id >= ids_.size()) {
				throw RepresentationError("cover tree index returned a node outside its record snapshot");
			}
			neighbors.push_back(neighbor_type{ids_[local_id], neighbor.second});
		}
		core::sort_neighbors(neighbors);
		if (neighbors.size() > k) {
			neighbors.resize(k);
		}
		return neighbors;
	}

	auto record_count() const -> std::size_t { return record_count_; }
	auto id(std::size_t position) const -> RecordId
	{
		validate_position(position);
		return ids_[position];
	}
	auto position_of(RecordId id) const -> std::size_t
	{
		return mtrc::position_of_record_id(ids_, id, "record id is outside the cover tree index");
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
	auto stats() const -> cover_tree_stats
	{
		cover_tree_stats result;
		result.nodes = record_count_;
		result.covering_validated = true;
		result.covering_valid = tree_ == nullptr || tree_->check_covering();
		return result;
	}

	auto diagnostics() const -> representation_diagnostics
	{
		representation_diagnostics result{representation_kind::cover_tree_index, exactness::exact,
										  materialization::materialized, update_mode::snapshot};
		result.space_version = space_->version();
		result.built_for_version = version_;
		result.stale = is_stale();
		result.records = record_count_;
		result.memory_bytes_estimate = ids_.size() * sizeof(RecordId) + records_.size() * sizeof(record_type);
		result.cache_key = cache_key_;
		result.metric_key = metric_key_;
		result.source_record_ids = ids_;
		if (result.stale) {
			result.warnings.push_back("cover tree index was built for an older metric-space version");
		}
		return result;
	}

  private:
	static auto require_true_metric_law() -> void
	{
		if constexpr (core::metric_traits<typename space_type::metric_type>::law != core::metric_law::metric) {
			throw RepresentationError("cover tree index requires metric_traits<Metric>::law == metric_law::metric");
		}
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
	std::string metric_key_;
	std::string cache_key_;
	std::vector<RecordId> ids_;
	std::vector<record_type> records_;
	std::unique_ptr<tree_type> tree_;
};

template <typename Space> auto cover_tree(const Space &space) -> CoverTreeIndex<Space>
{
	return CoverTreeIndex<Space>(space);
}

} // namespace mtrc::space::storage

#endif
