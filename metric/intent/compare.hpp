// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_COMPARE_HPP
#define _METRIC_INTENT_COMPARE_HPP

#include <type_traits>

#include "../core/concepts.hpp"
#include "../core/result.hpp"
#include "../operators/correlation.hpp"
#include "../representations/matrix_cache.hpp"
#include "../runtime/execution.hpp"
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
auto compare(const LeftSpace &left_space, const RightSpace &right_space, strategies::mgc strategy,
			 runtime::policy runtime_policy) -> CorrelationResult<double>
{
	runtime::require_exact_compare(runtime_policy);
	runtime::require_parallel_metric<typename LeftSpace::metric_type>(runtime_policy);
	runtime::require_parallel_metric<typename RightSpace::metric_type>(runtime_policy);

	if (runtime_policy.uses_materialization()) {
		representations::MatrixCache<LeftSpace> left_matrix(left_space);
		representations::MatrixCache<RightSpace> right_matrix(right_space);
		auto result = operators::mgc(left_matrix, right_matrix);
		result.left_representation = runtime::compare_representation(runtime_policy);
		result.right_representation = runtime::compare_representation(runtime_policy);
		return result;
	}

	auto result = compare(left_space, right_space, strategy);
	result.left_representation = runtime::compare_representation(runtime_policy);
	result.right_representation = runtime::compare_representation(runtime_policy);
	return result;
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto compare(const LeftSpace &left_space, const RightSpace &right_space) -> CorrelationResult<double>
{
	return compare(left_space, right_space, strategies::mgc{});
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto compare(const LeftSpace &left_space, const RightSpace &right_space, runtime::policy runtime_policy)
	-> CorrelationResult<double>
{
	return compare(left_space, right_space, strategies::mgc{}, runtime_policy);
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
auto correlate(const LeftSpace &left_space, const RightSpace &right_space, strategies::mgc strategy,
			   runtime::policy runtime_policy) -> CorrelationResult<double>
{
	return compare(left_space, right_space, strategy, runtime_policy);
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto correlate(const LeftSpace &left_space, const RightSpace &right_space) -> CorrelationResult<double>
{
	return correlate(left_space, right_space, strategies::mgc{});
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto correlate(const LeftSpace &left_space, const RightSpace &right_space, runtime::policy runtime_policy)
	-> CorrelationResult<double>
{
	return correlate(left_space, right_space, strategies::mgc{}, runtime_policy);
}

} // namespace metric

#endif
