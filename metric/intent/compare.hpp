// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_COMPARE_HPP
#define _METRIC_INTENT_COMPARE_HPP

#include <type_traits>

#include "../core/concepts.hpp"
#include "../core/result.hpp"
#include "../operators/correlation.hpp"
#include "../strategies/correlation.hpp"

namespace metric {

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto compare(const LeftSpace &left_space, const RightSpace &right_space, strategies::mgc)
	-> CorrelationResult<double>
{
	return operators::mgc(left_space, right_space);
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto compare(const LeftSpace &left_space, const RightSpace &right_space) -> CorrelationResult<double>
{
	return compare(left_space, right_space, strategies::mgc{});
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto correlate(const LeftSpace &left_space, const RightSpace &right_space, strategies::mgc strategy)
	-> CorrelationResult<double>
{
	return compare(left_space, right_space, strategy);
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto correlate(const LeftSpace &left_space, const RightSpace &right_space) -> CorrelationResult<double>
{
	return correlate(left_space, right_space, strategies::mgc{});
}

} // namespace metric

#endif
