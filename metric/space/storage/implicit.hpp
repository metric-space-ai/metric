// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_IMPLICIT_HPP
#define _METRIC_REPRESENTATIONS_IMPLICIT_HPP

#include <cstddef>

#include <metric/record/id.hpp>
#include "diagnostics.hpp"

namespace mtrc::space::storage {

template <typename Space> class LiveDistances {
  public:
	using space_type = Space;
	using distance_type = typename space_type::distance_type;

	explicit LiveDistances(const space_type &space) : space_(&space) {}

	auto distance(RecordId lhs, RecordId rhs) const -> distance_type { return space_->distance(lhs, rhs); }
	auto record_count() const -> std::size_t { return space_->size(); }
	auto id(std::size_t position) const -> RecordId { return space_->id(position); }
	auto position_of(RecordId id) const -> std::size_t { return space_->position_of(id); }
	auto contains(RecordId id) const -> bool { return space_->contains(id); }
	auto version() const -> std::size_t { return space_->version(); }
	auto is_stale() const -> bool { return false; }

	auto diagnostics() const -> representation_diagnostics
	{
		representation_diagnostics result{representation_kind::live_distances, exactness::exact,
										  materialization::lazy, update_mode::live};
		result.space_version = space_->version();
		result.built_for_version = space_->version();
		result.records = space_->size();
		return result;
	}

  private:
	const space_type *space_;
};

template <typename Space> auto implicit(const Space &space) -> LiveDistances<Space>
{
	return LiveDistances<Space>(space);
}

} // namespace mtrc::space::storage

#endif
