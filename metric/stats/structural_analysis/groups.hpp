// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_STRUCTURAL_ANALYSIS_GROUPS_HPP
#define _METRIC_STATS_STRUCTURAL_ANALYSIS_GROUPS_HPP

#include <cstddef>
#include <type_traits>

#include <metric/core/concepts.hpp>
#include <metric/core/result.hpp>
#include <metric/stats/structural_analysis/clustering.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/stats/structural_analysis/options.hpp>

namespace mtrc::stats::structural_analysis {

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, stats::structural_analysis::k_medoids_options strategy) -> ClusteringResult<typename Space::distance_type>
{
	return stats::structural_analysis::kmedoids(space, strategy.groups, strategy.max_iterations);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, stats::structural_analysis::k_medoids_options strategy, space::storage::policy runtime_policy)
	-> ClusteringResult<typename Space::distance_type>
{
	space::storage::require_exact_groups(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		space::storage::DistanceTable<Space> matrix(space);
		auto result = stats::structural_analysis::kmedoids(matrix, strategy.groups, strategy.max_iterations);
		result.representation = space::storage::group_representation(runtime_policy);
		return result;
	}

	auto result = stats::structural_analysis::kmedoids(space, strategy.groups, strategy.max_iterations);
	result.representation = space::storage::group_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, std::size_t groups) -> ClusteringResult<typename Space::distance_type>
{
	return find_groups(space, stats::structural_analysis::k_medoids_options(groups));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, std::size_t groups, space::storage::policy runtime_policy)
	-> ClusteringResult<typename Space::distance_type>
{
	return find_groups(space, stats::structural_analysis::k_medoids_options(groups), runtime_policy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, stats::structural_analysis::dbscan_options strategy) -> ClusteringResult<typename Space::distance_type>
{
	return stats::structural_analysis::dbscan(space, strategy.radius, strategy.min_points);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, stats::structural_analysis::dbscan_options strategy, space::storage::policy runtime_policy)
	-> ClusteringResult<typename Space::distance_type>
{
	space::storage::require_exact_groups(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		space::storage::DistanceTable<Space> matrix(space);
		auto result = stats::structural_analysis::dbscan(matrix, strategy.radius, strategy.min_points);
		result.representation = space::storage::group_representation(runtime_policy);
		return result;
	}

	auto result = stats::structural_analysis::dbscan(space, strategy.radius, strategy.min_points);
	result.representation = space::storage::group_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, stats::structural_analysis::affinity_propagation_options strategy)
	-> ClusteringResult<typename Space::distance_type>
{
	return stats::structural_analysis::affinity_propagation(space, strategy.preference, strategy.max_iterations, strategy.tolerance,
										   strategy.damping);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_groups(const Space &space, stats::structural_analysis::affinity_propagation_options strategy, space::storage::policy runtime_policy)
	-> ClusteringResult<typename Space::distance_type>
{
	space::storage::require_exact_groups(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		space::storage::DistanceTable<Space> matrix(space);
		auto result = stats::structural_analysis::affinity_propagation(matrix, strategy.preference, strategy.max_iterations,
													  strategy.tolerance, strategy.damping);
		result.representation = space::storage::group_representation(runtime_policy);
		return result;
	}

	auto result = stats::structural_analysis::affinity_propagation(space, strategy.preference, strategy.max_iterations,
												  strategy.tolerance, strategy.damping);
	result.representation = space::storage::group_representation(runtime_policy);
	return result;
}

} // namespace mtrc::stats::structural_analysis

namespace mtrc {
using stats::structural_analysis::find_groups;
} // namespace mtrc

#endif
