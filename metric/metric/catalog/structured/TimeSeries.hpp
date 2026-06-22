/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef _METRIC_METRIC_CATALOG_STRUCTURED_TIME_SERIES_HPP
#define _METRIC_METRIC_CATALOG_STRUCTURED_TIME_SERIES_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <metric/core/metric_traits.hpp>

namespace mtrc {

/**
 * Edit distance with Real Penalty (ERP) for scalar time series.
 *
 * Formula:
 *   D(i,j) = min(
 *     D(i-1,j-1) + |x_i - y_j|,
 *     D(i-1,j)   + |x_i - gap|,
 *     D(i,j-1)   + |gap - y_j|)
 *
 * This is the metric variant with absolute-value ground metric and a fixed gap
 * sentinel outside the admitted sample alphabet. The implementation enforces
 * that every sample value is finite and distinct from the configured gap value;
 * otherwise inserted/deleted gap-valued samples could collapse identity.
 */
template <typename V = double> class ERP {
  public:
	using value_type = V;
	using distance_type = value_type;

	static_assert(std::is_floating_point<V>::value, "ERP requires a floating-point value type");

	explicit ERP(value_type gap_value) : gap_(gap_value)
	{
		validate_gap();
	}

	template <typename Container> auto operator()(const Container &a, const Container &b) const -> distance_type
	{
		validate_gap();
		validate_series(a);
		validate_series(b);

		const auto n = a.size();
		const auto m = b.size();
		std::vector<value_type> previous(m + 1, value_type(0));
		std::vector<value_type> current(m + 1, value_type(0));

		for (std::size_t j = 1; j <= m; ++j) {
			previous[j] = previous[j - 1] + gap_cost(static_cast<value_type>(b[j - 1]));
		}

		for (std::size_t i = 1; i <= n; ++i) {
			const auto left = static_cast<value_type>(a[i - 1]);
			current[0] = previous[0] + gap_cost(left);
			for (std::size_t j = 1; j <= m; ++j) {
				const auto right = static_cast<value_type>(b[j - 1]);
				const auto substitute = previous[j - 1] + std::abs(left - right);
				const auto delete_left = previous[j] + gap_cost(left);
				const auto insert_right = current[j - 1] + gap_cost(right);
				current[j] = std::min({substitute, delete_left, insert_right});
			}
			std::swap(previous, current);
		}
		return previous[m];
	}

	auto gap_value() const -> value_type { return gap_; }

  private:
	value_type gap_;

	void validate_gap() const
	{
		if (!std::isfinite(gap_)) {
			throw std::invalid_argument("ERP requires a finite gap value");
		}
	}

	auto gap_cost(value_type value) const -> value_type { return std::abs(value - gap_); }

	template <typename Container> void validate_series(const Container &series) const
	{
		for (const auto &entry : series) {
			const auto value = static_cast<value_type>(entry);
			if (!std::isfinite(value)) {
				throw std::invalid_argument("ERP requires finite series values");
			}
			if (value == gap_) {
				throw std::invalid_argument("ERP sample values must be distinct from the gap value");
			}
		}
	}
};

} // namespace mtrc

namespace mtrc::core {

template <typename V> struct metric_traits<::mtrc::ERP<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::sequence;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::mtrc::ERP<V> &metric) -> std::string
	{
		return std::string("mtrc::ERP:gap=") + std::to_string(metric.gap_value());
	}
};

} // namespace mtrc::core

#endif
