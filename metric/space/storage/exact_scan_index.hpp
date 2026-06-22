// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_EXACT_SCAN_INDEX_HPP
#define _METRIC_REPRESENTATIONS_EXACT_SCAN_INDEX_HPP

#include <cstddef>
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

template <typename Space> class ExactScanIndex {
  public:
	using space_type = Space;
	using record_type = typename space_type::record_type;
	using distance_type = typename space_type::distance_type;
	using neighbor_type = Neighbor<distance_type>;

	explicit ExactScanIndex(const space_type &space)
		: space_(&space), record_count_(space.size()), version_(space.version()),
		  metric_key_(core::metric_cache_key(space.metric()))
	{
		ids_ = mtrc::record_ids(space);
		records_ = mtrc::records_for_record_ids(space, ids_);
		cache_key_ = representation_cache_key("exact_scan_index", metric_key_, version_, ids_);
	}

	auto knn(const record_type &query, std::size_t k) const -> std::vector<neighbor_type>
	{
		auto candidates = core::neighbor_candidates<distance_type>(
			record_count_, [this](std::size_t index) { return ids_[index]; },
			[this, &query](RecordId, std::size_t index) { return space_->metric()(query, records_[index]); });
		return core::take_nearest_neighbors(std::move(candidates), k);
	}

	auto record_count() const -> std::size_t { return record_count_; }
	auto id(std::size_t position) const -> RecordId
	{
		validate_position(position);
		return ids_[position];
	}
	auto position_of(RecordId id) const -> std::size_t
	{
		return mtrc::position_of_record_id(ids_, id, "record id is outside the exact scan index");
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

	auto diagnostics() const -> representation_diagnostics
	{
		representation_diagnostics result{representation_kind::live_distances, exactness::exact,
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
			result.warnings.push_back("exact scan index was built for an older metric-space version");
		}
		return result;
	}

  private:
	auto validate_position(std::size_t position) const -> void
	{
		if (position >= record_count_) {
			throw std::out_of_range("record position is outside the exact scan index");
		}
	}

	const space_type *space_;
	std::size_t record_count_;
	std::size_t version_;
	std::string metric_key_;
	std::string cache_key_;
	std::vector<RecordId> ids_;
	std::vector<record_type> records_;
};

template <typename Space> auto exact_scan(const Space &space) -> ExactScanIndex<Space>
{
	return ExactScanIndex<Space>(space);
}

} // namespace mtrc::space::storage

#endif
