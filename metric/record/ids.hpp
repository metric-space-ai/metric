// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RECORD_IDS_HPP
#define _METRIC_RECORD_IDS_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>

#include <metric/record/id.hpp>

namespace mtrc::record {

inline auto contains_record_id(const std::vector<RecordId> &ids, RecordId id) -> bool
{
	for (const auto current : ids) {
		if (current == id) {
			return true;
		}
	}
	return false;
}

inline auto position_of_record_id(const std::vector<RecordId> &ids, RecordId id, const char *not_found_message)
	-> std::size_t
{
	for (std::size_t position = 0; position < ids.size(); ++position) {
		if (ids[position] == id) {
			return position;
		}
	}
	throw std::out_of_range(not_found_message);
}

inline auto record_id_overlap_count(const std::vector<RecordId> &expected, const std::vector<RecordId> &candidates)
	-> std::size_t
{
	std::size_t matches = 0;
	for (const auto candidate : candidates) {
		if (contains_record_id(expected, candidate)) {
			++matches;
		}
	}
	return matches;
}

inline auto count_record_id(const std::vector<RecordId> &ids, RecordId id) -> std::size_t
{
	std::size_t occurrences = 0;
	for (const auto current : ids) {
		if (current == id) {
			++occurrences;
		}
	}
	return occurrences;
}

// Distinct record ids in first-seen order (stable de-duplication).
inline auto unique_record_ids(const std::vector<RecordId> &ids) -> std::vector<RecordId>
{
	std::vector<RecordId> unique;
	unique.reserve(ids.size());
	for (const auto id : ids) {
		if (!contains_record_id(unique, id)) {
			unique.push_back(id);
		}
	}
	return unique;
}

// Record ids that occur more than once, each reported once, in first-duplicate order.
inline auto duplicate_record_ids(const std::vector<RecordId> &ids) -> std::vector<RecordId>
{
	std::vector<RecordId> seen;
	std::vector<RecordId> duplicates;
	seen.reserve(ids.size());
	for (const auto id : ids) {
		if (contains_record_id(seen, id)) {
			if (!contains_record_id(duplicates, id)) {
				duplicates.push_back(id);
			}
		} else {
			seen.push_back(id);
		}
	}
	return duplicates;
}

inline auto has_duplicate_record_ids(const std::vector<RecordId> &ids) -> bool
{
	std::vector<RecordId> seen;
	seen.reserve(ids.size());
	for (const auto id : ids) {
		if (contains_record_id(seen, id)) {
			return true;
		}
		seen.push_back(id);
	}
	return false;
}

} // namespace mtrc::record

namespace mtrc {
using record::contains_record_id;
using record::count_record_id;
using record::duplicate_record_ids;
using record::has_duplicate_record_ids;
using record::position_of_record_id;
using record::record_id_overlap_count;
using record::unique_record_ids;
} // namespace mtrc

namespace mtrc::core {
using ::mtrc::record::contains_record_id;
using ::mtrc::record::count_record_id;
using ::mtrc::record::duplicate_record_ids;
using ::mtrc::record::has_duplicate_record_ids;
using ::mtrc::record::position_of_record_id;
using ::mtrc::record::record_id_overlap_count;
using ::mtrc::record::unique_record_ids;
} // namespace mtrc::core

#endif
