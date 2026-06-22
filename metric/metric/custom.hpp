// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_METRIC_CUSTOM_HPP
#define _METRIC_METRIC_CUSTOM_HPP

// Declare the metric law of a user callable so it becomes discoverable / admissible.
//
// mtrc::make_metric<Record>(callable) (in <metric/concepts.hpp>) wraps a callable
// with the default, no-claim law (metric_law::distance): safe, but NOT admitted by
// require_true_metric() or metric-geometry operators, and reported as non-metric by
// discovery. make_true_metric<Record>(callable) lets a caller who can vouch that the
// callable satisfies the metric axioms on its domain wrap it so the wrapper carries
// metric_law::metric via metric_traits and is admitted as a true metric.
//
//   auto d = mtrc::make_true_metric<MyRecord>(
//                [](const MyRecord& a, const MyRecord& b){ return my_true_distance(a, b); });
//   auto space = mtrc::space::build_checked(records, d);   // passes the true-metric gate

#include <type_traits>
#include <utility>

#include <metric/concepts.hpp>
#include <metric/core/metric_traits.hpp>

namespace mtrc::metric {

// A callable wrapped with an asserted true-metric law. Mirrors mtrc::Metric but
// carries metric_law::metric through metric_traits (see specialization below).
template <typename Record, typename Callable> class TrueMetric {
  public:
	using record_type = Record;
	using callable_type = Callable;

	explicit TrueMetric(Callable callable) : callable_(std::move(callable))
	{
		static_assert(is_metric_callable_v<callable_type, record_type>,
					  "mtrc::make_true_metric requires a callable accepting two records");
	}

	auto operator()(const Record &lhs, const Record &rhs) const { return callable_(lhs, rhs); }

	auto callable() const -> const Callable & { return callable_; }

  private:
	Callable callable_;
};

template <typename Record, typename Callable>
auto make_true_metric(Callable callable) -> TrueMetric<Record, typename std::decay<Callable>::type>
{
	return TrueMetric<Record, typename std::decay<Callable>::type>(std::move(callable));
}

} // namespace mtrc::metric

namespace mtrc::core {

// Propagate the asserted true-metric law so a wrapped custom metric is admitted
// by require_true_metric() and reported as a metric by discovery.
template <typename Record, typename Callable> struct metric_traits<mtrc::metric::TrueMetric<Record, Callable>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};

} // namespace mtrc::core

namespace mtrc {
using metric::make_true_metric;
} // namespace mtrc

#endif
