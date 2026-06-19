// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_MATRIX_CACHE_HPP
#define _METRIC_REPRESENTATIONS_MATRIX_CACHE_HPP

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <vector>

#include "../core/record_id.hpp"
#include "diagnostics.hpp"

namespace metric::representations {

enum class matrix_cache_mode {
	eager,
	lazy,
};

template <typename Space> class MatrixCache {
  public:
	using space_type = Space;
	using distance_type = typename space_type::distance_type;

	explicit MatrixCache(const space_type &space, matrix_cache_mode mode = matrix_cache_mode::eager)
		: space_(&space)
		, record_count_(space.size())
		, version_(space.version())
		, mode_(mode)
	{
		ids_.reserve(record_count_);
		for (std::size_t index = 0; index < record_count_; ++index) {
			ids_.push_back(space.id(index));
		}

		matrix_.resize(record_count_ * record_count_);
		if (mode_ == matrix_cache_mode::eager) {
			for (std::size_t lhs = 0; lhs < record_count_; ++lhs) {
				for (std::size_t rhs = 0; rhs < record_count_; ++rhs) {
					matrix_[offset(lhs, rhs)] = space.distance(ids_[lhs], ids_[rhs]);
					++cached_count_;
				}
			}
		}
	}

	auto distance(RecordId lhs, RecordId rhs) const -> distance_type
	{
		const auto lhs_position = position_of(lhs);
		const auto rhs_position = position_of(rhs);
		auto &cached = matrix_[offset(lhs_position, rhs_position)];
		if (cached.has_value()) {
			++hits_;
			return *cached;
		}
		++misses_;
		cached = space_->distance(lhs, rhs);
		++cached_count_;
		return *cached;
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
	auto cached_distances() const -> std::size_t { return cached_count_; }

	auto stats() const -> matrix_cache_stats
	{
		matrix_cache_stats result;
		result.hits = hits_;
		result.misses = misses_;
		result.fill_ratio = matrix_.empty() ? 1.0 : static_cast<double>(cached_count_) / static_cast<double>(matrix_.size());
		result.symmetric_storage = false;
		return result;
	}

	auto diagnostics() const -> representation_diagnostics
	{
		representation_diagnostics result{representation_kind::matrix_cache,
										  exactness::exact,
										  mode_ == matrix_cache_mode::eager ? materialization::materialized
																			: materialization::lazy,
										  update_mode::snapshot};
		result.space_version = space_->version();
		result.built_for_version = version_;
		result.stale = is_stale();
		result.records = record_count_;
		result.distance_evaluations = cached_count_;
		result.cached_distances = cached_count_;
		result.memory_bytes_estimate = matrix_.size() * sizeof(std::optional<distance_type>) +
									   ids_.size() * sizeof(RecordId);
		if (result.stale) {
			result.warnings.push_back("matrix cache was built for an older metric-space version");
		}
		return result;
	}

  private:
	auto offset(std::size_t lhs_position, std::size_t rhs_position) const -> std::size_t
	{
		return lhs_position * record_count_ + rhs_position;
	}

	auto validate_position(std::size_t position) const -> void
	{
		if (position >= record_count_) {
			throw std::out_of_range("record position is outside the matrix cache");
		}
	}

	const space_type *space_;
	std::size_t record_count_;
	std::size_t version_;
	matrix_cache_mode mode_;
	std::vector<RecordId> ids_;
	mutable std::vector<std::optional<distance_type>> matrix_;
	mutable std::size_t cached_count_{};
	mutable std::size_t hits_{};
	mutable std::size_t misses_{};
};

} // namespace metric::representations

#endif
