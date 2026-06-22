// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_METRIC_COMPOSE_HPP
#define _METRIC_METRIC_COMPOSE_HPP

// Combine two metrics over the same record into one metric.
//
//   weighted_sum(a, b, wa, wb)  ->  wa*a(x,y) + wb*b(x,y)   (wa, wb >= 0)
//   maximum(a, b)               ->  max(a(x,y), b(x,y))
//
// Both combinators preserve the metric law: a non-negative weighted sum, and a
// pointwise maximum, of two true metrics is again a true metric. The composed
// metric_traits law is metric_law::metric only when BOTH operands are true
// metrics, otherwise metric_law::distance -- so the composition never overstates
// its guarantee. Useful for structured records where two views (e.g. a numeric
// field metric and a sequence metric) are combined into one record metric.

#include <algorithm>
#include <type_traits>
#include <utility>

#include <metric/core/metric_traits.hpp>

namespace mtrc::metric {

namespace compose_detail {
template <core::metric_law A, core::metric_law B> constexpr auto combined_law() -> core::metric_law
{
	return (A == core::metric_law::metric && B == core::metric_law::metric) ? core::metric_law::metric
																			: core::metric_law::distance;
}
} // namespace compose_detail

template <typename MetricA, typename MetricB> class WeightedSum {
  public:
	WeightedSum(MetricA a, MetricB b, double weight_a, double weight_b)
		: a_(std::move(a)), b_(std::move(b)), weight_a_(weight_a), weight_b_(weight_b)
	{
	}

	template <typename Record> auto operator()(const Record &lhs, const Record &rhs) const -> double
	{
		return weight_a_ * static_cast<double>(a_(lhs, rhs)) + weight_b_ * static_cast<double>(b_(lhs, rhs));
	}

	auto first() const -> const MetricA & { return a_; }
	auto second() const -> const MetricB & { return b_; }

  private:
	MetricA a_;
	MetricB b_;
	double weight_a_;
	double weight_b_;
};

template <typename MetricA, typename MetricB> class Maximum {
  public:
	Maximum(MetricA a, MetricB b) : a_(std::move(a)), b_(std::move(b)) {}

	template <typename Record> auto operator()(const Record &lhs, const Record &rhs) const -> double
	{
		return (std::max)(static_cast<double>(a_(lhs, rhs)), static_cast<double>(b_(lhs, rhs)));
	}

  private:
	MetricA a_;
	MetricB b_;
};

template <typename MetricA, typename MetricB>
auto weighted_sum(MetricA a, MetricB b, double weight_a = 1.0, double weight_b = 1.0)
	-> WeightedSum<std::decay_t<MetricA>, std::decay_t<MetricB>>
{
	return WeightedSum<std::decay_t<MetricA>, std::decay_t<MetricB>>(std::move(a), std::move(b), weight_a, weight_b);
}

template <typename MetricA, typename MetricB>
auto maximum(MetricA a, MetricB b) -> Maximum<std::decay_t<MetricA>, std::decay_t<MetricB>>
{
	return Maximum<std::decay_t<MetricA>, std::decay_t<MetricB>>(std::move(a), std::move(b));
}

} // namespace mtrc::metric

namespace mtrc::core {

template <typename MetricA, typename MetricB> struct metric_traits<mtrc::metric::WeightedSum<MetricA, MetricB>> {
	static constexpr auto law = mtrc::metric::compose_detail::combined_law<metric_traits<MetricA>::law,
																		   metric_traits<MetricB>::law>();
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = metric_traits<MetricA>::thread_safe && metric_traits<MetricB>::thread_safe;
};

template <typename MetricA, typename MetricB> struct metric_traits<mtrc::metric::Maximum<MetricA, MetricB>> {
	static constexpr auto law = mtrc::metric::compose_detail::combined_law<metric_traits<MetricA>::law,
																		   metric_traits<MetricB>::law>();
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = metric_traits<MetricA>::thread_safe && metric_traits<MetricB>::thread_safe;
};

} // namespace mtrc::core

namespace mtrc {
using metric::maximum;
using metric::weighted_sum;
} // namespace mtrc

#endif
