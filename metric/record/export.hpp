// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RECORD_EXPORT_HPP
#define _METRIC_RECORD_EXPORT_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>

#include <metric/record/import.hpp>

// record::export sends records back out across the boundary as flat buffers or
// plain contiguous data. It is the inverse of record::import. It performs no
// metric, space, or statistics work.

namespace mtrc::record {

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

} // namespace mtrc::record

namespace mtrc {
using record::export_records_to_buffer;
using record::flatten_records;
using record::record_dimensions;
} // namespace mtrc

#endif
