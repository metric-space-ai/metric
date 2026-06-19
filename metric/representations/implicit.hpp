// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_IMPLICIT_HPP
#define _METRIC_REPRESENTATIONS_IMPLICIT_HPP

#include <cstddef>

#include "../core/record_id.hpp"

namespace metric::representations {

template <typename Space> class ImplicitDistanceProvider {
  public:
	using space_type = Space;
	using distance_type = typename space_type::distance_type;

	explicit ImplicitDistanceProvider(const space_type &space)
		: space_(&space)
		, version_(space.version())
	{
	}

	auto distance(RecordId lhs, RecordId rhs) const -> distance_type { return space_->distance(lhs, rhs); }
	auto record_count() const -> std::size_t { return space_->size(); }
	auto version() const -> std::size_t { return version_; }
	auto is_stale() const -> bool { return space_->version() != version_; }

  private:
	const space_type *space_;
	std::size_t version_;
};

} // namespace metric::representations

#endif
