// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RECORD_PRINT_HPP
#define _METRIC_RECORD_PRINT_HPP

#include <cstddef>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/record/compose.hpp>
#include <metric/record/id.hpp>

// record::print renders records and record ids as human-readable diagnostics.
// It understands scalars, vector records, record ids, and composed records.
// It is formatting only; it computes nothing about the metric space.

namespace mtrc::record {

namespace detail {

template <typename T> struct is_record_vector : std::false_type {};
template <typename T, typename A> struct is_record_vector<std::vector<T, A>> : std::true_type {};

template <typename T> struct is_composed_record : std::false_type {};
template <typename... Fields> struct is_composed_record<ComposedRecord<Fields...>> : std::true_type {};

} // namespace detail

template <typename T> auto write_record(std::ostream &os, const T &value) -> std::ostream &;

namespace detail {

template <typename Tuple, std::size_t... Indices>
auto write_tuple_fields(std::ostream &os, const Tuple &tuple, std::index_sequence<Indices...>) -> void
{
	((os << (Indices == 0 ? "" : " | "), write_record(os, std::get<Indices>(tuple))), ...);
}

} // namespace detail

template <typename T> auto write_record(std::ostream &os, const T &value) -> std::ostream &
{
	if constexpr (std::is_same<T, RecordId>::value) {
		os << '#' << value.index();
	} else if constexpr (detail::is_record_vector<T>::value) {
		os << '[';
		for (std::size_t index = 0; index < value.size(); ++index) {
			if (index != 0) {
				os << ", ";
			}
			write_record(os, value[index]);
		}
		os << ']';
	} else if constexpr (detail::is_composed_record<T>::value) {
		os << '(';
		detail::write_tuple_fields(os, value.fields, std::make_index_sequence<T::field_count>{});
		os << ')';
	} else {
		os << value;
	}
	return os;
}

template <typename T> auto print_record(std::ostream &os, const T &value) -> std::ostream &
{
	return write_record(os, value);
}

template <typename T> auto record_to_string(const T &value) -> std::string
{
	std::ostringstream stream;
	write_record(stream, value);
	return stream.str();
}

template <typename Record>
auto print_records(std::ostream &os, const std::vector<Record> &records) -> std::ostream &
{
	os << '[';
	for (std::size_t index = 0; index < records.size(); ++index) {
		if (index != 0) {
			os << ", ";
		}
		write_record(os, records[index]);
	}
	os << ']';
	return os;
}

template <typename Record> auto records_to_string(const std::vector<Record> &records) -> std::string
{
	std::ostringstream stream;
	print_records(stream, records);
	return stream.str();
}

inline auto print_record_ids(std::ostream &os, const std::vector<RecordId> &ids) -> std::ostream &
{
	return print_records(os, ids);
}

inline auto record_ids_to_string(const std::vector<RecordId> &ids) -> std::string
{
	return records_to_string(ids);
}

} // namespace mtrc::record

namespace mtrc {
using record::print_record;
using record::print_record_ids;
using record::print_records;
using record::record_ids_to_string;
using record::record_to_string;
using record::records_to_string;
using record::write_record;
} // namespace mtrc

#endif
