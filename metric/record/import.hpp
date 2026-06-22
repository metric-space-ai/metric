// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RECORD_IMPORT_HPP
#define _METRIC_RECORD_IMPORT_HPP

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <cmath>
#include <iterator>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// record::import brings external observations into the framework as records.
// It covers ordinary C++ containers and the flat, binding-facing buffers that
// language bindings hand over. It does not build metric spaces or pick metrics.

namespace mtrc::record {

struct DelimitedReadOptions {
	char delimiter = ',';
	char quote = '"';
	bool has_header = false;
	bool trim_fields = true;
	bool allow_empty_lines = true;
	bool require_uniform_dimension = true;
	bool require_finite = true;
};

using CsvReadOptions = DelimitedReadOptions;

// A flat, binding-facing buffer: record_count records laid out row-major, each
// with record_dim elements, in a single contiguous data vector. This is the
// canonical shape that NumPy-style bindings present to the native layer.
template <typename T> struct RecordBuffer {
	using value_type = T;

	std::vector<T> data;
	std::size_t record_count{};
	std::size_t record_dim{};

	auto element_count() const -> std::size_t { return record_count * record_dim; }
	auto empty() const -> bool { return record_count == 0; }
	auto consistent() const -> bool
	{
		if (record_dim != 0 && record_count > (std::numeric_limits<std::size_t>::max)() / record_dim) {
			return false;
		}
		return data.size() == record_count * record_dim;
	}
};

namespace detail {

inline auto is_ascii_space(char value) -> bool
{
	return std::isspace(static_cast<unsigned char>(value)) != 0;
}

inline auto trim_ascii(std::string field) -> std::string
{
	auto first = std::find_if(field.begin(), field.end(), [](char value) { return !is_ascii_space(value); });
	auto last = std::find_if(field.rbegin(), field.rend(), [](char value) { return !is_ascii_space(value); }).base();
	if (first >= last) {
		return {};
	}
	return std::string(first, last);
}

inline auto parse_delimited_line(const std::string &line, const DelimitedReadOptions &options,
								 std::size_t line_number) -> std::vector<std::string>
{
	if (options.delimiter == '\0' || options.quote == '\0' || options.delimiter == options.quote) {
		throw std::invalid_argument("mtrc::record::read_csv requires distinct non-zero delimiter and quote");
	}

	std::vector<std::string> fields;
	std::string field;
	bool in_quotes = false;

	for (std::size_t index = 0; index < line.size(); ++index) {
		const char value = line[index];
		if (in_quotes) {
			if (value == options.quote) {
				if (index + 1 < line.size() && line[index + 1] == options.quote) {
					field.push_back(options.quote);
					++index;
				} else {
					in_quotes = false;
				}
			} else {
				field.push_back(value);
			}
		} else if (value == options.quote && field.empty()) {
			in_quotes = true;
		} else if (value == options.delimiter) {
			fields.push_back(options.trim_fields ? trim_ascii(std::move(field)) : std::move(field));
			field.clear();
		} else {
			field.push_back(value);
		}
	}

	if (in_quotes) {
		std::ostringstream message;
		message << "mtrc::record::read_csv unterminated quoted field at line " << line_number;
		throw std::invalid_argument(message.str());
	}

	fields.push_back(options.trim_fields ? trim_ascii(std::move(field)) : std::move(field));
	return fields;
}

inline auto is_empty_record_line(const std::vector<std::string> &fields) -> bool
{
	return fields.size() == 1 && fields.front().empty();
}

template <typename T>
auto parse_record_field(const std::string &field, std::size_t line_number, std::size_t column_number,
						const DelimitedReadOptions &options) -> T
{
	if (field.empty()) {
		std::ostringstream message;
		message << "mtrc::record::read_csv empty field at line " << line_number << ", column " << column_number;
		throw std::invalid_argument(message.str());
	}

	if constexpr (std::is_same<typename std::decay<T>::type, std::string>::value) {
		(void)options;
		return field;
	} else {
		std::istringstream stream(field);
		T value{};
		if (!(stream >> value)) {
			std::ostringstream message;
			message << "mtrc::record::read_csv could not parse field at line " << line_number << ", column "
					<< column_number;
			throw std::invalid_argument(message.str());
		}
		stream >> std::ws;
		if (!stream.eof()) {
			std::ostringstream message;
			message << "mtrc::record::read_csv could not parse field at line " << line_number << ", column "
					<< column_number;
			throw std::invalid_argument(message.str());
		}
		if constexpr (std::is_floating_point<T>::value) {
			if (options.require_finite && !std::isfinite(value)) {
				std::ostringstream message;
				message << "mtrc::record::read_csv non-finite value at line " << line_number << ", column "
						<< column_number;
				throw std::invalid_argument(message.str());
			}
		}
		return value;
	}
}

// record_count * record_dim, guarded against std::size_t overflow. Without this
// guard a pathological record_dim can wrap the product (mod 2^64) and let a
// too-small buffer pass shape validation, producing out-of-bounds reads later.
inline auto checked_buffer_element_count(std::size_t record_count, std::size_t record_dim, const char *message)
	-> std::size_t
{
	if (record_dim != 0 && record_count > (std::numeric_limits<std::size_t>::max)() / record_dim) {
		throw std::invalid_argument(message);
	}
	return record_count * record_dim;
}

} // namespace detail

template <typename T>
auto make_record_buffer(std::vector<T> data, std::size_t record_count, std::size_t record_dim) -> RecordBuffer<T>
{
	const auto expected = detail::checked_buffer_element_count(
		record_count, record_dim, "mtrc::record::make_record_buffer record_count * record_dim overflows std::size_t");
	if (data.size() != expected) {
		throw std::invalid_argument(
			"mtrc::record::make_record_buffer data size does not match record_count * record_dim");
	}
	return RecordBuffer<T>{std::move(data), record_count, record_dim};
}

// Import records by copying any forward-iterable container of records.
template <typename Container>
auto import_records(const Container &records)
	-> std::vector<typename std::decay<decltype(*std::begin(std::declval<const Container &>()))>::type>
{
	using record_type = typename std::decay<decltype(*std::begin(std::declval<const Container &>()))>::type;
	return std::vector<record_type>(std::begin(records), std::end(records));
}

// Import vector records from a flat, row-major binding buffer.
template <typename T>
auto import_records_from_buffer(const std::vector<T> &buffer, std::size_t record_count, std::size_t record_dim)
	-> std::vector<std::vector<T>>
{
	const auto expected = detail::checked_buffer_element_count(
		record_count, record_dim,
		"mtrc::record::import_records_from_buffer record_count * record_dim overflows std::size_t");
	if (buffer.size() != expected) {
		throw std::invalid_argument(
			"mtrc::record::import_records_from_buffer buffer size does not match record_count * record_dim");
	}

	std::vector<std::vector<T>> records;
	records.reserve(record_count);
	for (std::size_t record_index = 0; record_index < record_count; ++record_index) {
		const auto offset = record_index * record_dim;
		records.emplace_back(buffer.begin() + static_cast<std::ptrdiff_t>(offset),
							 buffer.begin() + static_cast<std::ptrdiff_t>(offset + record_dim));
	}
	return records;
}

template <typename T> auto import_records_from_buffer(const RecordBuffer<T> &buffer) -> std::vector<std::vector<T>>
{
	return import_records_from_buffer(buffer.data, buffer.record_count, buffer.record_dim);
}

template <typename T = double>
auto read_delimited_records(std::istream &input, DelimitedReadOptions options = {}) -> std::vector<std::vector<T>>
{
	std::vector<std::vector<T>> records;
	std::string line;
	std::size_t line_number = 0;
	std::size_t expected_dimension = 0;
	bool dimension_set = false;

	while (std::getline(input, line)) {
		++line_number;
		if (options.has_header) {
			options.has_header = false;
			continue;
		}

		auto fields = detail::parse_delimited_line(line, options, line_number);
		if (options.allow_empty_lines && detail::is_empty_record_line(fields)) {
			continue;
		}
		if (options.require_uniform_dimension) {
			if (!dimension_set) {
				expected_dimension = fields.size();
				dimension_set = true;
			} else if (fields.size() != expected_dimension) {
				std::ostringstream message;
				message << "mtrc::record::read_csv row at line " << line_number << " has " << fields.size()
						<< " fields; expected " << expected_dimension;
				throw std::invalid_argument(message.str());
			}
		}

		std::vector<T> record;
		record.reserve(fields.size());
		for (std::size_t column = 0; column < fields.size(); ++column) {
			record.push_back(detail::parse_record_field<T>(fields[column], line_number, column + 1, options));
		}
		records.push_back(std::move(record));
	}

	if (input.bad()) {
		throw std::runtime_error("mtrc::record::read_csv failed while reading stream");
	}
	return records;
}

template <typename T = double>
auto read_delimited_records(const std::string &path, DelimitedReadOptions options = {}) -> std::vector<std::vector<T>>
{
	std::ifstream input(path);
	if (!input) {
		throw std::runtime_error("mtrc::record::read_csv could not open file: " + path);
	}
	return read_delimited_records<T>(input, options);
}

template <typename T = double>
auto read_csv(const std::string &path, CsvReadOptions options = {}) -> std::vector<std::vector<T>>
{
	options.delimiter = ',';
	return read_delimited_records<T>(path, options);
}

template <typename T = double>
auto read_csv(std::istream &input, CsvReadOptions options = {}) -> std::vector<std::vector<T>>
{
	options.delimiter = ',';
	return read_delimited_records<T>(input, options);
}

template <typename T = double>
auto read_tsv(const std::string &path, DelimitedReadOptions options = {}) -> std::vector<std::vector<T>>
{
	options.delimiter = '\t';
	return read_delimited_records<T>(path, options);
}

template <typename T = double>
auto read_tsv(std::istream &input, DelimitedReadOptions options = {}) -> std::vector<std::vector<T>>
{
	options.delimiter = '\t';
	return read_delimited_records<T>(input, options);
}

} // namespace mtrc::record

namespace mtrc {
template <typename T> using RecordBuffer = record::RecordBuffer<T>;
using record::CsvReadOptions;
using record::DelimitedReadOptions;
using record::import_records;
using record::import_records_from_buffer;
using record::make_record_buffer;
using record::read_csv;
using record::read_delimited_records;
using record::read_tsv;
} // namespace mtrc

#endif
