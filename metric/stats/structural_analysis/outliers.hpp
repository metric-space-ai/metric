// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_OUTLIERS_HPP
#define _METRIC_INTENT_OUTLIERS_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#include <metric/core/concepts.hpp>
#include <metric/core/result.hpp>
#include <metric/stats/structural_analysis/clustering.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/stats/structural_analysis/options.hpp>

namespace mtrc::stats::structural_analysis {
namespace outlier_detail {

template <typename Provider>
auto outlier_result_from_groups(const Provider &provider,
								const ClusteringResult<typename Provider::distance_type> &groups)
	-> OutlierResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	const auto references = ::mtrc::non_noise_record_ids(provider, groups);

	auto outliers = core::scored_outliers<distance_type>(groups.noise_records, [&](RecordId id) {
		return references.empty()
			? core::nearest_other_record_distance_or(provider, id, distance_type{})
			: core::nearest_distance_to_record_ids(provider, id, references, "outlier references must not be empty");
	});

	return core::make_outlier_result(std::move(outliers), groups.record_count, groups.cluster_count, groups.noise_count,
									 "dbscan_noise", groups.representation);
}

} // namespace outlier_detail

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto find_outliers(const Provider &provider, stats::structural_analysis::dbscan_options strategy)
	-> OutlierResult<typename Provider::distance_type>
{
	const auto groups = stats::structural_analysis::dbscan(provider, strategy.radius, strategy.min_points);
	return outlier_detail::outlier_result_from_groups(provider, groups);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, stats::structural_analysis::dbscan_options strategy) -> OutlierResult<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	auto result = find_outliers(provider, strategy);
	result.representation = "metric_space";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, stats::structural_analysis::dbscan_options strategy, space::storage::policy runtime_policy)
	-> OutlierResult<typename Space::distance_type>
{
	space::storage::require_exact_outliers(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		space::storage::DistanceTable<Space> matrix(space);
		auto result = find_outliers(matrix, strategy);
		result.representation = space::storage::outlier_representation(runtime_policy);
		return result;
	}

	auto result = find_outliers(space, strategy);
	result.representation = space::storage::outlier_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, double radius, std::size_t min_points)
	-> OutlierResult<typename Space::distance_type>
{
	return find_outliers(space, stats::structural_analysis::dbscan_options(radius, min_points));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, double radius, std::size_t min_points, space::storage::policy runtime_policy)
	-> OutlierResult<typename Space::distance_type>
{
	return find_outliers(space, stats::structural_analysis::dbscan_options(radius, min_points), runtime_policy);
}

} // namespace mtrc::stats::structural_analysis

namespace mtrc {
using stats::structural_analysis::find_outliers;
} // namespace mtrc

#endif
