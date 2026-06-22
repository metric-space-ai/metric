// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_BENCHMARKS_COUNTING_METRIC_HPP
#define _METRIC_BENCHMARKS_COUNTING_METRIC_HPP

#include <cstddef>
#include <memory>
#include <utility>

namespace mtrc::benchmarks {

// Deterministic instrumentation wrapper.
//
// CountingMetric wraps any metric callable and counts how many times its
// operator() is invoked. The counter is *shared* (a std::shared_ptr), so every
// copy the engine makes -- make_space stores the metric by value, and the
// storage/representation machinery may copy it again -- increments the SAME
// counter. A benchmark can therefore OBSERVE the number of domain-metric
// evaluations a workflow actually performs instead of asserting it by
// construction. The wrapper forwards the wrapped metric's exact return type, so
// MetricSpace's deduced distance_type is unchanged.
//
// The count is a deterministic integer, independent of wall-clock and platform,
// which keeps any artifact built from it byte-stable.
template <typename Metric> class CountingMetric {
  public:
	explicit CountingMetric(Metric metric)
		: metric_(std::move(metric)), counter_(std::make_shared<std::size_t>(0))
	{
	}

	CountingMetric(Metric metric, std::shared_ptr<std::size_t> counter)
		: metric_(std::move(metric)), counter_(std::move(counter))
	{
	}

	template <typename Lhs, typename Rhs>
	auto operator()(const Lhs &lhs, const Rhs &rhs) const -> decltype(std::declval<const Metric &>()(lhs, rhs))
	{
		++(*counter_);
		return metric_(lhs, rhs);
	}

	// Number of operator() invocations observed across every copy of this wrapper.
	auto count() const -> std::size_t { return *counter_; }

	// Reset the shared counter (affects every copy that shares it).
	void reset() const { *counter_ = 0; }

	// The shared counter handle, so callers can keep observing after the wrapper
	// has been moved into a space.
	auto counter() const -> std::shared_ptr<std::size_t> { return counter_; }

	auto metric() const -> const Metric & { return metric_; }

  private:
	Metric metric_;
	std::shared_ptr<std::size_t> counter_;
};

template <typename Metric> auto make_counting_metric(Metric metric) -> CountingMetric<Metric>
{
	return CountingMetric<Metric>(std::move(metric));
}

} // namespace mtrc::benchmarks

#endif
