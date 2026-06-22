// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RECORD_COMPOSE_HPP
#define _METRIC_RECORD_COMPOSE_HPP

#include <cstddef>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// record::compose builds composed (mixed) records out of several typed fields,
// e.g. a record holding a text part, a histogram part, and a time-series part.
// It only assembles and accesses the data structure. Comparing composed records
// with a true metric is mtrc::metric::compose, not this header.

namespace mtrc::record {

template <typename... Fields> struct ComposedRecord {
	using tuple_type = std::tuple<Fields...>;

	tuple_type fields;

	static constexpr std::size_t field_count = sizeof...(Fields);

	template <std::size_t Index>
	auto field() const -> const typename std::tuple_element<Index, tuple_type>::type &
	{
		return std::get<Index>(fields);
	}

	template <std::size_t Index> auto field() -> typename std::tuple_element<Index, tuple_type>::type &
	{
		return std::get<Index>(fields);
	}

	friend auto operator==(const ComposedRecord &lhs, const ComposedRecord &rhs) -> bool
	{
		return lhs.fields == rhs.fields;
	}
	friend auto operator!=(const ComposedRecord &lhs, const ComposedRecord &rhs) -> bool { return !(lhs == rhs); }
};

// Compose a single record from heterogeneous field values.
template <typename... Fields>
auto compose_record(Fields &&...values) -> ComposedRecord<typename std::decay<Fields>::type...>
{
	return ComposedRecord<typename std::decay<Fields>::type...>{std::make_tuple(std::forward<Fields>(values)...)};
}

namespace detail {

template <typename First, typename... Rest>
auto columns_row_count(const First &first, const Rest &...rest) -> std::size_t
{
	const std::size_t count = first.size();
	const bool equal = ((rest.size() == count) && ...);
	if (!equal) {
		throw std::invalid_argument("mtrc::record::compose_records requires equal-length field columns");
	}
	return count;
}

} // namespace detail

// Compose a collection of records column-wise: given one column (vector) per
// field, produce a row-wise collection of composed records. All columns must
// have equal length.
template <typename... Columns>
auto compose_records(const std::vector<Columns> &...columns) -> std::vector<ComposedRecord<Columns...>>
{
	static_assert(sizeof...(Columns) > 0, "mtrc::record::compose_records requires at least one field column");
	const std::size_t count = detail::columns_row_count(columns...);
	std::vector<ComposedRecord<Columns...>> records;
	records.reserve(count);
	for (std::size_t row = 0; row < count; ++row) {
		records.push_back(ComposedRecord<Columns...>{std::make_tuple(columns[row]...)});
	}
	return records;
}

} // namespace mtrc::record

namespace mtrc {
template <typename... Fields> using ComposedRecord = record::ComposedRecord<Fields...>;
using record::compose_record;
using record::compose_records;
} // namespace mtrc

#endif
