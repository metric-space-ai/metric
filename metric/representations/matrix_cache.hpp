// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_MATRIX_CACHE_HPP
#define _METRIC_REPRESENTATIONS_MATRIX_CACHE_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "../core/record_id.hpp"
#include "diagnostics.hpp"

namespace metric::representations {

template <typename Space> class MatrixCache {
  public:
	using space_type = Space;
	using distance_type = typename space_type::distance_type;

	explicit MatrixCache(const space_type &space)
		: space_(&space)
		, record_count_(space.size())
		, version_(space.version())
	{
		ids_.reserve(record_count_);
		for (std::size_t index = 0; index < record_count_; ++index) {
			ids_.push_back(space.id(index));
		}

		matrix_.reserve(record_count_ * record_count_);
		for (std::size_t lhs = 0; lhs < record_count_; ++lhs) {
			for (std::size_t rhs = 0; rhs < record_count_; ++rhs) {
				matrix_.push_back(space.distance(ids_[lhs], ids_[rhs]));
			}
		}
	}

	auto distance(RecordId lhs, RecordId rhs) const -> distance_type
	{
		const auto lhs_position = position_of(lhs);
		const auto rhs_position = position_of(rhs);
		return matrix_[lhs_position * record_count_ + rhs_position];
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
		throw std::out_of_range("record id is outside the matrix cache");
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
	auto cached_distances() const -> std::size_t { return matrix_.size(); }

	auto stats() const -> matrix_cache_stats
	{
		matrix_cache_stats result;
		result.fill_ratio = 1.0;
		result.symmetric_storage = false;
		return result;
	}

	auto diagnostics() const -> representation_diagnostics
	{
		representation_diagnostics result{representation_kind::matrix_cache,
										  exactness::exact,
										  materialization::materialized,
										  update_mode::snapshot};
		result.space_version = space_->version();
		result.built_for_version = version_;
		result.stale = is_stale();
		result.records = record_count_;
		result.distance_evaluations = matrix_.size();
		result.cached_distances = matrix_.size();
		result.memory_bytes_estimate = matrix_.size() * sizeof(distance_type) + ids_.size() * sizeof(RecordId);
		if (result.stale) {
			result.warnings.push_back("matrix cache was built for an older metric-space version");
		}
		return result;
	}

  private:
	auto validate_position(std::size_t position) const -> void
	{
		if (position >= record_count_) {
			throw std::out_of_range("record position is outside the matrix cache");
		}
	}

	const space_type *space_;
	std::size_t record_count_;
	std::size_t version_;
	std::vector<RecordId> ids_;
	std::vector<distance_type> matrix_;
};

} // namespace metric::representations

#endif
