// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RECORD_IMPORT_HPP
#define _METRIC_RECORD_IMPORT_HPP

#include <cstddef>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// record::import brings external observations into the framework as records.
// It covers ordinary C++ containers and the flat, binding-facing buffers that
// language bindings hand over. It does not build metric spaces or pick metrics.

namespace mtrc::record {

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

} // namespace mtrc::record

namespace mtrc {
template <typename T> using RecordBuffer = record::RecordBuffer<T>;
using record::import_records;
using record::import_records_from_buffer;
using record::make_record_buffer;
} // namespace mtrc

#endif
