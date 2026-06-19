// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_CONCEPTS_HPP
#define _METRIC_CORE_CONCEPTS_HPP

#include <type_traits>
#include <utility>

#include "metric_traits.hpp"

namespace metric::core {

template <typename Metric, typename Record, typename = void> struct MetricCallable : std::false_type {
};

template <typename Metric, typename Record>
struct MetricCallable<Metric, Record,
					  std::void_t<decltype(std::declval<const Metric &>()(std::declval<const Record &>(),
																		   std::declval<const Record &>()))>>
	: std::true_type {
};

template <typename Metric, typename Record>
constexpr bool MetricCallable_v = MetricCallable<Metric, Record>::value;

template <typename Metric, typename Record>
using metric_result_t = decltype(std::declval<const Metric &>()(std::declval<const Record &>(),
																std::declval<const Record &>()));

template <typename Space, typename = void> struct MetricSpaceLike : std::false_type {
};

template <typename Space>
struct MetricSpaceLike<Space,
					   std::void_t<typename Space::record_type, typename Space::metric_type,
								   typename Space::distance_type, decltype(std::declval<const Space &>().size()),
								   decltype(std::declval<const Space &>().version())>> : std::true_type {
};

template <typename Space>
constexpr bool MetricSpaceLike_v = MetricSpaceLike<Space>::value;

} // namespace metric::core

namespace metric {
using core::metric_result_t;
using core::MetricCallable;
using core::MetricCallable_v;
using core::MetricSpaceLike;
using core::MetricSpaceLike_v;
} // namespace metric

#endif
