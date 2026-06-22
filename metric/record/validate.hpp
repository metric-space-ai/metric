// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RECORD_VALIDATE_HPP
#define _METRIC_RECORD_VALIDATE_HPP

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

#include <metric/record/id.hpp>
#include <metric/record/ids.hpp>

// record::validate checks record-level invariants before records enter a space:
// non-empty record sets, unique record ids, uniform dimensions for composed or
// buffer-facing records, and consistent binding-buffer shapes. It never asserts
// anything about metrics, distances, or space structure.

namespace mtrc::record {

// Non-throwing summary of a record-id set.
struct RecordIdReport {
	std::size_t count{};
	std::size_t unique_count{};
	std::size_t duplicate_count{};
	bool empty{true};
	bool has_duplicates{false};
};

inline auto inspect_record_ids(const std::vector<RecordId> &ids) -> RecordIdReport
{
	RecordIdReport report;
	report.count = ids.size();
	report.unique_count = unique_record_ids(ids).size();
	report.duplicate_count = duplicate_record_ids(ids).size();
	report.empty = ids.empty();
	report.has_duplicates = report.duplicate_count != 0;
	return report;
}

inline auto validate_unique_record_ids(
	const std::vector<RecordId> &ids,
	const char *message = "mtrc::record::validate_unique_record_ids found duplicate record ids") -> void
{
	if (has_duplicate_record_ids(ids)) {
		throw std::invalid_argument(message);
	}
}

template <typename Container>
auto validate_records_non_empty(
	const Container &records,
	const char *message = "mtrc::record::validate_records_non_empty received an empty record set") -> void
{
	if (records.empty()) {
		throw std::invalid_argument(message);
	}
}

// Confirm every vector record has the same dimension and return it (0 for an
// empty record set). Required for composed records and buffer export.
template <typename T>
auto validate_uniform_record_dimension(
	const std::vector<std::vector<T>> &records,
	const char *message = "mtrc::record::validate_uniform_record_dimension requires uniform record dimension")
	-> std::size_t
{
	if (records.empty()) {
		return 0;
	}
	const std::size_t record_dim = records.front().size();
	for (const auto &record : records) {
		if (record.size() != record_dim) {
			throw std::invalid_argument(message);
		}
	}
	return record_dim;
}

inline auto validate_buffer_shape(
	std::size_t buffer_size, std::size_t record_count, std::size_t record_dim,
	const char *message =
		"mtrc::record::validate_buffer_shape buffer size does not match record_count * record_dim") -> void
{
	// Guard the product against std::size_t overflow before comparing.
	if (record_dim != 0 && record_count > (std::numeric_limits<std::size_t>::max)() / record_dim) {
		throw std::invalid_argument(message);
	}
	if (buffer_size != record_count * record_dim) {
		throw std::invalid_argument(message);
	}
}

} // namespace mtrc::record

namespace mtrc {
using record::inspect_record_ids;
using record::RecordIdReport;
using record::validate_buffer_shape;
using record::validate_records_non_empty;
using record::validate_uniform_record_dimension;
using record::validate_unique_record_ids;
} // namespace mtrc

#endif
