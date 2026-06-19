// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_MATRIX_CACHE_HPP
#define _METRIC_REPRESENTATIONS_MATRIX_CACHE_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "../core/record_id.hpp"

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
		matrix_.reserve(record_count_ * record_count_);
		for (std::size_t lhs = 0; lhs < record_count_; ++lhs) {
			for (std::size_t rhs = 0; rhs < record_count_; ++rhs) {
				matrix_.push_back(space.distance(RecordId::from_index(lhs), RecordId::from_index(rhs)));
			}
		}
	}

	auto distance(RecordId lhs, RecordId rhs) const -> distance_type
	{
		validate(lhs);
		validate(rhs);
		return matrix_[lhs.index() * record_count_ + rhs.index()];
	}

	auto record_count() const -> std::size_t { return record_count_; }
	auto version() const -> std::size_t { return version_; }
	auto is_stale() const -> bool { return space_->version() != version_; }

  private:
	auto validate(RecordId id) const -> void
	{
		if (id.index() >= record_count_) {
			throw std::out_of_range("record id is outside the matrix cache");
		}
	}

	const space_type *space_;
	std::size_t record_count_;
	std::size_t version_;
	std::vector<distance_type> matrix_;
};

} // namespace metric::representations

#endif
