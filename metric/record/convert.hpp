// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RECORD_CONVERT_HPP
#define _METRIC_RECORD_CONVERT_HPP

#include <cstddef>
#include <vector>

// record::convert changes the C++ representation of a record value without
// touching its meaning. It is element-wise type conversion only: no metric,
// no distance, no space construction, no statistics.

namespace mtrc::record {

// Convert a single scalar-like record value to a different representation.
template <typename Target, typename Source> auto convert_record_value(const Source &value) -> Target
{
	return static_cast<Target>(value);
}

// Convert the elements of a vector record to a different element type.
template <typename Target, typename Source>
auto convert_record(const std::vector<Source> &record) -> std::vector<Target>
{
	std::vector<Target> converted;
	converted.reserve(record.size());
	for (const auto &element : record) {
		converted.push_back(static_cast<Target>(element));
	}
	return converted;
}

// Convert a collection of vector records element-wise.
template <typename Target, typename Source>
auto convert_records(const std::vector<std::vector<Source>> &records) -> std::vector<std::vector<Target>>
{
	std::vector<std::vector<Target>> converted;
	converted.reserve(records.size());
	for (const auto &record : records) {
		converted.push_back(convert_record<Target>(record));
	}
	return converted;
}

} // namespace mtrc::record

namespace mtrc {
using record::convert_record;
using record::convert_record_value;
using record::convert_records;
} // namespace mtrc

#endif
