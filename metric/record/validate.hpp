// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RECORD_VALIDATE_HPP
#define _METRIC_RECORD_VALIDATE_HPP

#include <cstddef>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
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

struct RecordColumnIssue {
	std::size_t field_index{};
	std::size_t expected_count{};
	std::size_t actual_count{};
	std::string message;
};

struct RecordColumnReport {
	std::size_t field_count{};
	std::size_t row_count{};
	bool empty{true};
	bool consistent{true};
	std::vector<std::size_t> field_sizes;
	std::vector<RecordColumnIssue> issues;

	auto ok() const -> bool { return consistent && issues.empty(); }
	auto issue_count() const -> std::size_t { return issues.size(); }
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

namespace detail {

inline auto append_record_column_issue(RecordColumnReport &report, std::size_t field_index,
									   std::size_t expected_count, std::size_t actual_count) -> void
{
	std::ostringstream message;
	message << "field " << field_index << " has " << actual_count << " records; expected " << expected_count;
	report.issues.push_back(RecordColumnIssue{field_index, expected_count, actual_count, message.str()});
	report.consistent = false;
}

template <typename First, typename... Rest>
auto inspect_record_columns_impl(const First &first, const Rest &...rest) -> RecordColumnReport
{
	RecordColumnReport report;
	report.field_count = 1 + sizeof...(Rest);
	report.row_count = first.size();
	report.empty = first.empty();
	report.field_sizes.reserve(report.field_count);
	report.field_sizes.push_back(first.size());

	std::size_t field_index = 1;
	auto inspect_next = [&](const auto &column) {
		const auto size = column.size();
		report.field_sizes.push_back(size);
		if (size != report.row_count) {
			append_record_column_issue(report, field_index, report.row_count, size);
		}
		if (size != 0) {
			report.empty = false;
		}
		++field_index;
	};
	(inspect_next(rest), ...);
	return report;
}

inline auto record_column_report_message(const RecordColumnReport &report) -> std::string
{
	std::ostringstream message;
	message << "mtrc::record field columns are not aligned";
	for (const auto &issue : report.issues) {
		message << "; " << issue.message;
	}
	return message.str();
}

} // namespace detail

// Inspect field columns before composing mixed records. The first column defines
// the expected row count; every later column must have the same number of
// records. This is a non-throwing diagnostic for UI/binding code.
template <typename First, typename... Rest>
auto inspect_record_columns(const std::vector<First> &first, const std::vector<Rest> &...rest) -> RecordColumnReport
{
	return detail::inspect_record_columns_impl(first, rest...);
}

template <typename First, typename... Rest>
auto validate_record_columns(const std::vector<First> &first, const std::vector<Rest> &...rest) -> RecordColumnReport
{
	auto report = inspect_record_columns(first, rest...);
	if (!report.ok()) {
		throw std::invalid_argument(detail::record_column_report_message(report));
	}
	return report;
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
using record::inspect_record_columns;
using record::RecordColumnIssue;
using record::RecordColumnReport;
using record::RecordIdReport;
using record::validate_buffer_shape;
using record::validate_record_columns;
using record::validate_records_non_empty;
using record::validate_uniform_record_dimension;
using record::validate_unique_record_ids;
} // namespace mtrc

#endif
