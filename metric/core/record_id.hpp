// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_RECORD_ID_HPP
#define _METRIC_CORE_RECORD_ID_HPP

#include <cstddef>

namespace metric::core {

class RecordId {
  public:
	constexpr RecordId() = default;

	static constexpr auto from_index(std::size_t index) -> RecordId { return RecordId(index); }

	constexpr auto index() const -> std::size_t { return index_; }

	friend constexpr auto operator==(RecordId lhs, RecordId rhs) -> bool { return lhs.index_ == rhs.index_; }
	friend constexpr auto operator!=(RecordId lhs, RecordId rhs) -> bool { return !(lhs == rhs); }
	friend constexpr auto operator<(RecordId lhs, RecordId rhs) -> bool { return lhs.index_ < rhs.index_; }

  private:
	explicit constexpr RecordId(std::size_t index)
		: index_(index)
	{
	}

	std::size_t index_{};
};

} // namespace metric::core

namespace metric {
using RecordId = core::RecordId;
} // namespace metric

#endif
