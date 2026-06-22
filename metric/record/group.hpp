// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RECORD_GROUP_HPP
#define _METRIC_RECORD_GROUP_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/record/id.hpp>

// record::group partitions a record-id set by an explicit key the caller
// supplies. It is plain bookkeeping with deterministic ordering. It is NOT
// clustering or any metric-driven structure discovery (that lives in
// mtrc::stats); no distances are computed here.

namespace mtrc::record {

template <typename Key> struct RecordGrouping {
	using key_type = Key;

	std::vector<Key> keys;                     // distinct keys, in first-seen order
	std::vector<std::vector<RecordId>> groups; // record ids per key, parallel to keys

	auto group_count() const -> std::size_t { return keys.size(); }
	auto empty() const -> bool { return keys.empty(); }
};

namespace detail {

template <typename Key>
auto record_grouping_bucket(RecordGrouping<Key> &grouping, const Key &key) -> std::size_t
{
	for (std::size_t index = 0; index < grouping.keys.size(); ++index) {
		if (grouping.keys[index] == key) {
			return index;
		}
	}
	grouping.keys.push_back(key);
	grouping.groups.emplace_back();
	return grouping.keys.size() - 1;
}

} // namespace detail

// Group record ids by a key function key_of(RecordId) -> Key. Keys appear in
// first-seen order; ids keep their input order within each group.
template <typename KeyOf>
auto group_record_ids_by_key(const std::vector<RecordId> &ids, KeyOf key_of)
	-> RecordGrouping<typename std::decay<decltype(key_of(std::declval<RecordId>()))>::type>
{
	using key_type = typename std::decay<decltype(key_of(std::declval<RecordId>()))>::type;
	RecordGrouping<key_type> grouping;
	for (const auto id : ids) {
		key_type key = key_of(id);
		const auto bucket = detail::record_grouping_bucket(grouping, key);
		grouping.groups[bucket].push_back(id);
	}
	return grouping;
}

// Group record ids by a parallel vector of precomputed keys (keys[i] is the key
// of ids[i]).
template <typename Key>
auto group_record_ids(const std::vector<RecordId> &ids, const std::vector<Key> &keys) -> RecordGrouping<Key>
{
	if (ids.size() != keys.size()) {
		throw std::invalid_argument("mtrc::record::group_record_ids requires ids and keys of equal length");
	}
	RecordGrouping<Key> grouping;
	for (std::size_t index = 0; index < ids.size(); ++index) {
		const auto bucket = detail::record_grouping_bucket(grouping, keys[index]);
		grouping.groups[bucket].push_back(ids[index]);
	}
	return grouping;
}

} // namespace mtrc::record

namespace mtrc {
template <typename Key> using RecordGrouping = record::RecordGrouping<Key>;
using record::group_record_ids;
using record::group_record_ids_by_key;
} // namespace mtrc

#endif
