// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_GROUPS_HPP
#define _METRIC_INTENT_GROUPS_HPP

#include <cstddef>
#include <type_traits>

#include "../core/concepts.hpp"
#include "../core/result.hpp"
#include "../operators/clustering.hpp"
#include "../representations/matrix_cache.hpp"
#include "../runtime/execution.hpp"
#include "../strategies/clustering.hpp"

namespace metric::intent {

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, strategies::k_medoids strategy) -> ClusteringResult<typename Space::distance_type>
{
	return operators::kmedoids(space, strategy.groups, strategy.max_iterations);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, strategies::k_medoids strategy, runtime::policy runtime_policy)
	-> ClusteringResult<typename Space::distance_type>
{
	runtime::require_exact_groups(runtime_policy);
	runtime::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		representations::MatrixCache<Space> matrix(space);
		auto result = operators::kmedoids(matrix, strategy.groups, strategy.max_iterations);
		result.representation = runtime::group_representation(runtime_policy);
		return result;
	}

	auto result = operators::kmedoids(space, strategy.groups, strategy.max_iterations);
	result.representation = runtime::group_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, std::size_t groups) -> ClusteringResult<typename Space::distance_type>
{
	return find_groups(space, strategies::k_medoids(groups));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, std::size_t groups, runtime::policy runtime_policy)
	-> ClusteringResult<typename Space::distance_type>
{
	return find_groups(space, strategies::k_medoids(groups), runtime_policy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, strategies::dbscan strategy) -> ClusteringResult<typename Space::distance_type>
{
	return operators::dbscan(space, strategy.radius, strategy.min_points);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, strategies::dbscan strategy, runtime::policy runtime_policy)
	-> ClusteringResult<typename Space::distance_type>
{
	runtime::require_exact_groups(runtime_policy);
	runtime::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		representations::MatrixCache<Space> matrix(space);
		auto result = operators::dbscan(matrix, strategy.radius, strategy.min_points);
		result.representation = runtime::group_representation(runtime_policy);
		return result;
	}

	auto result = operators::dbscan(space, strategy.radius, strategy.min_points);
	result.representation = runtime::group_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, strategies::affinity_propagation strategy)
	-> ClusteringResult<typename Space::distance_type>
{
	return operators::affinity_propagation(space, strategy.preference, strategy.max_iterations, strategy.tolerance,
										   strategy.damping);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, strategies::affinity_propagation strategy, runtime::policy runtime_policy)
	-> ClusteringResult<typename Space::distance_type>
{
	runtime::require_exact_groups(runtime_policy);
	runtime::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		representations::MatrixCache<Space> matrix(space);
		auto result = operators::affinity_propagation(matrix, strategy.preference, strategy.max_iterations,
													  strategy.tolerance, strategy.damping);
		result.representation = runtime::group_representation(runtime_policy);
		return result;
	}

	auto result =
		operators::affinity_propagation(space, strategy.preference, strategy.max_iterations, strategy.tolerance,
										strategy.damping);
	result.representation = runtime::group_representation(runtime_policy);
	return result;
}

} // namespace metric::intent

namespace metric {
using intent::find_groups;
} // namespace metric

#endif
