/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef _METRIC_METRIC_CATALOG_DISTRIBUTION_EMPIRICAL_HPP
#define _METRIC_METRIC_CATALOG_DISTRIBUTION_EMPIRICAL_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/core/metric_traits.hpp>

namespace mtrc::detail {

template <typename V, typename Sample> auto sorted_empirical_sample(const Sample &sample, const char *name) -> std::vector<V>
{
	if (sample.empty()) {
		throw std::invalid_argument(std::string(name) + " requires non-empty empirical samples");
	}
	std::vector<V> values;
	values.reserve(sample.size());
	for (const auto &entry : sample) {
		const auto value = static_cast<V>(entry);
		if (!std::isfinite(value)) {
			throw std::invalid_argument(std::string(name) + " requires finite sample values");
		}
		values.push_back(value);
	}
	std::sort(values.begin(), values.end());
	return values;
}

template <typename V> auto next_support_value(const std::vector<V> &a, std::size_t i, const std::vector<V> &b, std::size_t j)
	-> V
{
	if (i == a.size()) {
		return b[j];
	}
	if (j == b.size()) {
		return a[i];
	}
	return std::min(a[i], b[j]);
}

template <typename V> void advance_tie_block(const std::vector<V> &values, std::size_t &index, V support)
{
	while (index < values.size() && values[index] == support) {
		++index;
	}
}

} // namespace mtrc::detail

namespace mtrc {

/**
 * Tie-safe Kolmogorov-Smirnov metric on empirical distributions.
 *
 * The record is the empirical distribution represented by a finite non-empty
 * sample. Sample order is ignored, tied values are grouped exactly, and the
 * distance is the true sup norm of the two empirical step CDFs.
 */
template <typename V = double> struct EmpiricalKolmogorovSmirnov {
	using value_type = V;
	using distance_type = value_type;

	template <typename Sample> auto operator()(const Sample &a, const Sample &b) const -> distance_type
	{
		const auto left = detail::sorted_empirical_sample<value_type>(a, "EmpiricalKolmogorovSmirnov");
		const auto right = detail::sorted_empirical_sample<value_type>(b, "EmpiricalKolmogorovSmirnov");

		std::size_t i = 0;
		std::size_t j = 0;
		value_type cdf_left = 0;
		value_type cdf_right = 0;
		value_type distance = 0;
		while (i < left.size() || j < right.size()) {
			const auto support = detail::next_support_value(left, i, right, j);
			detail::advance_tie_block(left, i, support);
			detail::advance_tie_block(right, j, support);
			cdf_left = static_cast<value_type>(i) / static_cast<value_type>(left.size());
			cdf_right = static_cast<value_type>(j) / static_cast<value_type>(right.size());
			distance = std::max(distance, std::abs(cdf_left - cdf_right));
		}
		return distance;
	}
};

/**
 * Root Cramer / CDF-L2 metric on empirical distributions.
 *
 * Formula:
 *   d(F, G) = sqrt(integral_R (F(x) - G(x))^2 dx)
 *
 * This is the root metric, not the squared Cramer-von-Mises statistic. The
 * integral is exact for empirical step CDFs: it sums each support interval
 * length times the squared constant CDF difference on that interval.
 */
template <typename V = double> struct EmpiricalCramer {
	using value_type = V;
	using distance_type = value_type;

	template <typename Sample> auto operator()(const Sample &a, const Sample &b) const -> distance_type
	{
		const auto left = detail::sorted_empirical_sample<value_type>(a, "EmpiricalCramer");
		const auto right = detail::sorted_empirical_sample<value_type>(b, "EmpiricalCramer");

		std::size_t i = 0;
		std::size_t j = 0;
		value_type cdf_left = 0;
		value_type cdf_right = 0;
		value_type previous = 0;
		value_type total = 0;
		bool have_previous = false;

		while (i < left.size() || j < right.size()) {
			const auto support = detail::next_support_value(left, i, right, j);
			if (have_previous) {
				total += (support - previous) * (cdf_left - cdf_right) * (cdf_left - cdf_right);
			}
			detail::advance_tie_block(left, i, support);
			detail::advance_tie_block(right, j, support);
			cdf_left = static_cast<value_type>(i) / static_cast<value_type>(left.size());
			cdf_right = static_cast<value_type>(j) / static_cast<value_type>(right.size());
			previous = support;
			have_previous = true;
		}
		return std::sqrt(std::max(value_type(0), total));
	}
};

} // namespace mtrc

namespace mtrc::core {

template <typename V> struct metric_traits<::mtrc::EmpiricalKolmogorovSmirnov<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::structured;
	static constexpr bool thread_safe = true;
};

template <typename V> struct metric_traits<::mtrc::EmpiricalCramer<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::structured;
	static constexpr bool thread_safe = true;
};

} // namespace mtrc::core

#endif
