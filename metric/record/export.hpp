// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RECORD_EXPORT_HPP
#define _METRIC_RECORD_EXPORT_HPP

#include <cstddef>
#include <fstream>
#include <iomanip>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/record/import.hpp>

// record::export sends records back out across the boundary as flat buffers or
// plain contiguous data. It is the inverse of record::import. It performs no
// metric, space, or statistics work.

namespace mtrc::record {

struct DelimitedWriteOptions {
	char delimiter = ',';
	char quote = '"';
	bool newline_at_end = true;
	std::vector<std::string> header;
};

using CsvWriteOptions = DelimitedWriteOptions;

namespace detail {

inline auto needs_quoted_field(const std::string &field, const DelimitedWriteOptions &options) -> bool
{
	if (field.empty()) {
		return false;
	}
	if (field.front() == ' ' || field.back() == ' ') {
		return true;
	}
	for (const auto value : field) {
		if (value == options.delimiter || value == options.quote || value == '\n' || value == '\r') {
			return true;
		}
	}
	return false;
}

inline auto quote_delimited_field(const std::string &field, const DelimitedWriteOptions &options) -> std::string
{
	if (!needs_quoted_field(field, options)) {
		return field;
	}

	std::string quoted;
	quoted.reserve(field.size() + 2);
	quoted.push_back(options.quote);
	for (const auto value : field) {
		if (value == options.quote) {
			quoted.push_back(options.quote);
		}
		quoted.push_back(value);
	}
	quoted.push_back(options.quote);
	return quoted;
}

template <typename T> auto format_delimited_field(const T &value, const DelimitedWriteOptions &options) -> std::string
{
	if constexpr (std::is_same<typename std::decay<T>::type, std::string>::value) {
		return quote_delimited_field(value, options);
	} else if constexpr (std::is_same<typename std::decay<T>::type, const char *>::value ||
						 std::is_same<typename std::decay<T>::type, char *>::value) {
		return quote_delimited_field(std::string(value), options);
	} else {
		std::ostringstream stream;
		if constexpr (std::is_floating_point<typename std::decay<T>::type>::value) {
			stream << std::setprecision(std::numeric_limits<typename std::decay<T>::type>::max_digits10);
		}
		stream << value;
		return stream.str();
	}
}

inline auto validate_write_options(const DelimitedWriteOptions &options) -> void
{
	if (options.delimiter == '\0' || options.quote == '\0' || options.delimiter == options.quote) {
		throw std::invalid_argument("mtrc::record::write_csv requires distinct non-zero delimiter and quote");
	}
}

inline auto write_header(std::ostream &output, const DelimitedWriteOptions &options) -> void
{
	for (std::size_t column = 0; column < options.header.size(); ++column) {
		if (column != 0) {
			output << options.delimiter;
		}
		output << quote_delimited_field(options.header[column], options);
	}
	output << '\n';
}

} // namespace detail

// Per-record dimensions (sizes). Works for ragged record collections and lets
// binding code reconstruct row boundaries from a flattened buffer.
template <typename T>
auto record_dimensions(const std::vector<std::vector<T>> &records) -> std::vector<std::size_t>
{
	std::vector<std::size_t> dimensions;
	dimensions.reserve(records.size());
	for (const auto &record : records) {
		dimensions.push_back(record.size());
	}
	return dimensions;
}

// Concatenate vector records into a single row-major data vector. Ragged
// records are allowed; pair with record_dimensions() to recover the shape.
template <typename T> auto flatten_records(const std::vector<std::vector<T>> &records) -> std::vector<T>
{
	std::size_t total = 0;
	for (const auto &record : records) {
		total += record.size();
	}

	std::vector<T> data;
	data.reserve(total);
	for (const auto &record : records) {
		data.insert(data.end(), record.begin(), record.end());
	}
	return data;
}

// Export a collection of vector records into a flat, row-major binding buffer.
// All records must share the same dimension.
template <typename T> auto export_records_to_buffer(const std::vector<std::vector<T>> &records) -> RecordBuffer<T>
{
	const std::size_t record_count = records.size();
	const std::size_t record_dim = records.empty() ? 0 : records.front().size();

	for (const auto &record : records) {
		if (record.size() != record_dim) {
			throw std::invalid_argument(
				"mtrc::record::export_records_to_buffer requires uniform record dimension");
		}
	}

	return RecordBuffer<T>{flatten_records(records), record_count, record_dim};
}

template <typename T>
auto write_delimited_records(std::ostream &output, const std::vector<std::vector<T>> &records,
							 DelimitedWriteOptions options = {}) -> void
{
	detail::validate_write_options(options);
	if (!options.header.empty()) {
		detail::write_header(output, options);
	}

	for (std::size_t row = 0; row < records.size(); ++row) {
		const auto &record = records[row];
		for (std::size_t column = 0; column < record.size(); ++column) {
			if (column != 0) {
				output << options.delimiter;
			}
			output << detail::format_delimited_field(record[column], options);
		}
		if (row + 1 < records.size() || options.newline_at_end) {
			output << '\n';
		}
	}
}

template <typename T>
auto write_delimited_records(const std::string &path, const std::vector<std::vector<T>> &records,
							 DelimitedWriteOptions options = {}) -> void
{
	std::ofstream output(path);
	if (!output) {
		throw std::runtime_error("mtrc::record::write_csv could not open file: " + path);
	}
	write_delimited_records(output, records, std::move(options));
	if (!output) {
		throw std::runtime_error("mtrc::record::write_csv failed while writing file: " + path);
	}
}

template <typename T>
auto write_csv(std::ostream &output, const std::vector<std::vector<T>> &records, CsvWriteOptions options = {}) -> void
{
	options.delimiter = ',';
	write_delimited_records(output, records, std::move(options));
}

template <typename T>
auto write_csv(const std::string &path, const std::vector<std::vector<T>> &records,
			   CsvWriteOptions options = {}) -> void
{
	options.delimiter = ',';
	write_delimited_records(path, records, std::move(options));
}

template <typename T>
auto write_tsv(std::ostream &output, const std::vector<std::vector<T>> &records,
			   DelimitedWriteOptions options = {}) -> void
{
	options.delimiter = '\t';
	write_delimited_records(output, records, std::move(options));
}

template <typename T>
auto write_tsv(const std::string &path, const std::vector<std::vector<T>> &records,
			   DelimitedWriteOptions options = {}) -> void
{
	options.delimiter = '\t';
	write_delimited_records(path, records, std::move(options));
}

} // namespace mtrc::record

namespace mtrc {
using record::CsvWriteOptions;
using record::DelimitedWriteOptions;
using record::export_records_to_buffer;
using record::flatten_records;
using record::record_dimensions;
using record::write_csv;
using record::write_delimited_records;
using record::write_tsv;
} // namespace mtrc

#endif
