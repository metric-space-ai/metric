// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MODIFY_DENSITY_FILTER_HPP
#define _METRIC_MODIFY_DENSITY_FILTER_HPP

#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/record/id.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/stats/structural_analysis/clustering.hpp>
#include <metric/stats/structural_analysis/groups.hpp>
#include <metric/stats/structural_analysis/options.hpp>

namespace mtrc::modify::resample {
namespace detail {

inline auto density_filter_strategy(std::string algorithm, bool exact) -> std::string
{
	if (exact || algorithm == "dbscan") {
		return "dbscan_density_filter";
	}
	const std::string suffix = "_unassigned";
	if (algorithm.size() >= suffix.size() &&
		algorithm.compare(algorithm.size() - suffix.size(), suffix.size(), suffix) == 0) {
		algorithm.erase(algorithm.size() - suffix.size());
	}
	return algorithm + "_density_filter";
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto density_filter_from_groups(const Space &space, const ClusteringResult<typename Space::distance_type> &groups)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	using target_space_type = MetricSpace<typename Space::record_type, typename Space::metric_type>;

	auto kept_ids = ::mtrc::assigned_record_ids(space, groups);
	auto records = ::mtrc::records_for_record_ids(space, kept_ids);

	auto lineage = ::mtrc::one_to_one_lineage(std::move(kept_ids));
	target_space_type derived_space(std::move(records), space.metric());
	const auto filter_strategy = density_filter_strategy(groups.algorithm, groups.exact);
	// This is a density/outlier filter: it removes DBSCAN-unassigned records. It
	// does not apply inverse metric dynamics to individual records.
	return core::make_mapping_result(
		std::move(derived_space), std::move(lineage.source_records), std::move(lineage.representative_records),
		space.size(), false, "density_filter", filter_strategy, groups.representation,
		core::metric_traits<typename Space::metric_type>::law, false,
		"density-based record filtering by removing DBSCAN-unassigned records; kept records are an unmodified subset "
		"of the source under the source metric");
}

} // namespace detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto density_filter(const Space &space, stats::structural_analysis::dbscan_options strategy)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	const auto groups = stats::structural_analysis::find_groups(space, strategy);
	return detail::density_filter_from_groups(space, groups);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto density_filter(const Space &space, stats::structural_analysis::dbscan_options strategy,
					space::storage::policy runtime_policy)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.is_approximate()) {
		const auto groups = stats::structural_analysis::find_groups(space, strategy, runtime_policy);
		return detail::density_filter_from_groups(space, groups);
	}

	space::storage::require_exact_density_filter(runtime_policy);
	auto runtime = space::storage::runtime_guard(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		return space::storage::with_materialized_distance_provider(space, runtime_policy, "density_filter",
			[&space, strategy, runtime](const auto &provider, const auto &plan) {
				auto groups = stats::structural_analysis::group_detail::guarded_dbscan(
					provider, strategy.radius, strategy.min_points, runtime);
				groups.representation = space::storage::materialized_operator_representation(plan);
				return detail::density_filter_from_groups(space, groups);
			});
	}

	auto groups = stats::structural_analysis::group_detail::guarded_dbscan_space(
		space, strategy.radius, strategy.min_points, runtime);
	auto result = detail::density_filter_from_groups(space, groups);
	result.representation = space::storage::density_filter_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto density_filter(const Space &space, double radius, std::size_t min_points)
	-> decltype(density_filter(space, stats::structural_analysis::dbscan_options(radius, min_points)))
{
	return density_filter(space, stats::structural_analysis::dbscan_options(radius, min_points));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto density_filter(const Space &space, double radius, std::size_t min_points, space::storage::policy runtime_policy)
	-> decltype(density_filter(space, stats::structural_analysis::dbscan_options(radius, min_points), runtime_policy))
{
	return density_filter(space, stats::structural_analysis::dbscan_options(radius, min_points), runtime_policy);
}

} // namespace mtrc::modify::resample

namespace mtrc {
using modify::resample::density_filter;
} // namespace mtrc

#endif
