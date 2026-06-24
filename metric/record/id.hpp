// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RECORD_ID_HPP
#define _METRIC_RECORD_ID_HPP

#include <cstddef>
#include <functional>

namespace mtrc::record {

class RecordId {
  public:
	constexpr RecordId() = default;

	static constexpr auto from_index(std::size_t index) -> RecordId { return RecordId(index); }

	constexpr auto index() const -> std::size_t { return index_; }

	friend constexpr auto operator==(RecordId lhs, RecordId rhs) -> bool { return lhs.index_ == rhs.index_; }
	friend constexpr auto operator!=(RecordId lhs, RecordId rhs) -> bool { return !(lhs == rhs); }
	friend constexpr auto operator<(RecordId lhs, RecordId rhs) -> bool { return lhs.index_ < rhs.index_; }

  private:
	explicit constexpr RecordId(std::size_t index) : index_(index) {}

	std::size_t index_{};
};

} // namespace mtrc::record

namespace mtrc {
using RecordId = record::RecordId;
} // namespace mtrc

namespace mtrc::core {
using ::mtrc::record::RecordId;
} // namespace mtrc::core

namespace std {

template <> struct hash<mtrc::record::RecordId> {
	auto operator()(mtrc::record::RecordId id) const noexcept -> std::size_t
	{
		return std::hash<std::size_t>{}(id.index());
	}
};

} // namespace std

#endif
