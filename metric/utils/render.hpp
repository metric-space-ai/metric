// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_UTILS_RENDER_HPP
#define _METRIC_UTILS_RENDER_HPP

// Small, dependency-free presentation helpers for making finite-metric-space
// data and results legible: export a distance matrix or an embedding to CSV,
// and print a quick ASCII histogram of a value distribution. These operate on
// plain std containers (no coupling to storage internals) so they compose with
// pairwise_distances(), a MappingResult's coordinates, or a DistanceDistribution
// without pulling the whole engine in.

#include <cstddef>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace mtrc::render {

// Write a 2D numeric table (e.g. a pairwise distance matrix, or embedding
// coordinates) as CSV. Optional row/column headers; when omitted, 0-based
// indices are used for the column header and the row index is the first cell.
template <typename T>
auto write_csv(std::ostream &os, const std::vector<std::vector<T>> &rows,
			   const std::vector<std::string> &column_headers = {}, bool index_column = true) -> void
{
	const std::size_t columns = rows.empty() ? 0 : rows.front().size();
	if (index_column || !column_headers.empty()) {
		if (index_column) {
			os << "index";
		}
		for (std::size_t column = 0; column < columns; ++column) {
			if (index_column || column != 0) {
				os << ',';
			}
			if (column < column_headers.size()) {
				os << column_headers[column];
			} else {
				os << column;
			}
		}
		os << '\n';
	}
	for (std::size_t row = 0; row < rows.size(); ++row) {
		if (index_column) {
			os << row;
		}
		for (std::size_t column = 0; column < rows[row].size(); ++column) {
			if (index_column || column != 0) {
				os << ',';
			}
			os << rows[row][column];
		}
		os << '\n';
	}
}

// Convenience: return the CSV as a string.
template <typename T>
auto to_csv(const std::vector<std::vector<T>> &rows, const std::vector<std::string> &column_headers = {},
			bool index_column = true) -> std::string
{
	std::ostringstream out;
	write_csv(out, rows, column_headers, index_column);
	return out.str();
}

// Print a compact fixed-bin ASCII histogram of a value distribution (e.g. the
// off-diagonal entries of a distance matrix), so a user can eyeball the shape of
// a finite metric space without a plotting stack. Empty input prints nothing.
template <typename T>
auto write_histogram(std::ostream &os, const std::vector<T> &values, std::size_t bins = 10,
					  std::size_t width = 40) -> void
{
	if (values.empty() || bins == 0) {
		return;
	}
	double lo = static_cast<double>(values.front());
	double hi = lo;
	for (const auto &value : values) {
		const double v = static_cast<double>(value);
		if (v < lo) {
			lo = v;
		}
		if (v > hi) {
			hi = v;
		}
	}
	const double span = (hi > lo) ? (hi - lo) : 1.0;
	std::vector<std::size_t> counts(bins, 0);
	for (const auto &value : values) {
		double position = (static_cast<double>(value) - lo) / span * static_cast<double>(bins);
		std::size_t bin = (position >= static_cast<double>(bins)) ? bins - 1 : static_cast<std::size_t>(position);
		++counts[bin];
	}
	std::size_t peak = 0;
	for (const auto count : counts) {
		if (count > peak) {
			peak = count;
		}
	}
	for (std::size_t bin = 0; bin < bins; ++bin) {
		const double bin_lo = lo + span * static_cast<double>(bin) / static_cast<double>(bins);
		const std::size_t bar = (peak == 0) ? 0 : counts[bin] * width / peak;
		os << '[';
		std::ostringstream label;
		label << bin_lo;
		os << label.str() << "] ";
		for (std::size_t i = 0; i < bar; ++i) {
			os << '#';
		}
		os << ' ' << counts[bin] << '\n';
	}
}

} // namespace mtrc::render

#endif
