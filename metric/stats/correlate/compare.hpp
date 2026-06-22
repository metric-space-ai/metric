// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_COMPARE_HPP
#define _METRIC_INTENT_COMPARE_HPP

#include <type_traits>

#include <metric/core/concepts.hpp>
#include <metric/core/result.hpp>
#include <metric/stats/correlate/correlation.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/stats/correlate/options.hpp>

namespace mtrc {

// compare()/correlate() test DEPENDENCE between two paired finite metric spaces. They
// are not metrics and return a CorrelationResult carrying a single dependence statistic
// (the MGC sample statistic in [-1, 1]); no p-value is produced. The two spaces must be
// aligned by observation and share the same record count. See
// stats/correlate/correlation.hpp for the full statistical contract.
template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto compare(const LeftSpace &left_space, const RightSpace &right_space, stats::correlate::mgc_options) -> CorrelationResult<double>
{
	return stats::correlate::mgc(left_space, right_space);
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto compare(const LeftSpace &left_space, const RightSpace &right_space, stats::correlate::mgc_options strategy,
			 space::storage::policy runtime_policy) -> CorrelationResult<double>
{
	space::storage::require_exact_compare(runtime_policy);
	space::storage::require_parallel_metric<typename LeftSpace::metric_type>(runtime_policy);
	space::storage::require_parallel_metric<typename RightSpace::metric_type>(runtime_policy);

	if (runtime_policy.uses_materialization()) {
		space::storage::DistanceTable<LeftSpace> left_matrix(left_space);
		space::storage::DistanceTable<RightSpace> right_matrix(right_space);
		auto result = stats::correlate::mgc(left_matrix, right_matrix);
		result.left_representation = space::storage::compare_representation(runtime_policy);
		result.right_representation = space::storage::compare_representation(runtime_policy);
		return result;
	}

	auto result = compare(left_space, right_space, strategy);
	result.left_representation = space::storage::compare_representation(runtime_policy);
	result.right_representation = space::storage::compare_representation(runtime_policy);
	return result;
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto compare(const LeftSpace &left_space, const RightSpace &right_space) -> CorrelationResult<double>
{
	return compare(left_space, right_space, stats::correlate::mgc_options{});
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto compare(const LeftSpace &left_space, const RightSpace &right_space, space::storage::policy runtime_policy)
	-> CorrelationResult<double>
{
	return compare(left_space, right_space, stats::correlate::mgc_options{}, runtime_policy);
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto correlate(const LeftSpace &left_space, const RightSpace &right_space, stats::correlate::mgc_options strategy)
	-> CorrelationResult<double>
{
	return compare(left_space, right_space, strategy);
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto correlate(const LeftSpace &left_space, const RightSpace &right_space, stats::correlate::mgc_options strategy,
			   space::storage::policy runtime_policy) -> CorrelationResult<double>
{
	return compare(left_space, right_space, strategy, runtime_policy);
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto correlate(const LeftSpace &left_space, const RightSpace &right_space) -> CorrelationResult<double>
{
	return correlate(left_space, right_space, stats::correlate::mgc_options{});
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto correlate(const LeftSpace &left_space, const RightSpace &right_space, space::storage::policy runtime_policy)
	-> CorrelationResult<double>
{
	return correlate(left_space, right_space, stats::correlate::mgc_options{}, runtime_policy);
}

} // namespace mtrc

#endif
