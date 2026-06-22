// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_DENOISE_HPP
#define _METRIC_INTENT_DENOISE_HPP

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/record/id.hpp>
#include <metric/core/result.hpp>
#include <metric/stats/structural_analysis/clustering.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/stats/structural_analysis/options.hpp>

namespace mtrc::modify::resample {
namespace detail {

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto denoise_from_groups(const Space &space, const ClusteringResult<typename Space::distance_type> &groups)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	using target_space_type = MetricSpace<typename Space::record_type, typename Space::metric_type>;

	auto kept_ids = ::mtrc::non_noise_record_ids(space, groups);
	auto records = ::mtrc::records_for_record_ids(space, kept_ids);

	auto lineage = ::mtrc::one_to_one_lineage(std::move(kept_ids));
	target_space_type derived_space(std::move(records), space.metric());
	// Denoise corrects an unevenly sampled finite space by dropping density-defined
	// noise records. The kept records are an unmodified subset of the source and
	// retain the source metric unchanged, so the metric law is preserved.
	return core::make_mapping_result(
		std::move(derived_space), std::move(lineage.source_records), std::move(lineage.representative_records),
		space.size(), false, "density_denoise", "dbscan_noise_filter", groups.representation,
		core::metric_traits<typename Space::metric_type>::law, false,
		"uneven-sampling correction by removing density-defined noise records; kept records are an unmodified "
		"subset of the source under the source metric");
}

} // namespace detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto denoise(const Space &space, stats::structural_analysis::dbscan_options strategy)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	const auto groups = stats::structural_analysis::dbscan(space, strategy.radius, strategy.min_points);
	return detail::denoise_from_groups(space, groups);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto denoise(const Space &space, stats::structural_analysis::dbscan_options strategy, space::storage::policy runtime_policy)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	space::storage::require_exact_denoise(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		space::storage::DistanceTable<Space> matrix(space);
		auto groups = stats::structural_analysis::dbscan(matrix, strategy.radius, strategy.min_points);
		groups.representation = space::storage::denoise_representation(runtime_policy);
		return detail::denoise_from_groups(space, groups);
	}

	auto result = denoise(space, strategy);
	result.representation = space::storage::denoise_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto denoise(const Space &space, double radius, std::size_t min_points)
	-> decltype(denoise(space, stats::structural_analysis::dbscan_options(radius, min_points)))
{
	return denoise(space, stats::structural_analysis::dbscan_options(radius, min_points));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto denoise(const Space &space, double radius, std::size_t min_points, space::storage::policy runtime_policy)
	-> decltype(denoise(space, stats::structural_analysis::dbscan_options(radius, min_points), runtime_policy))
{
	return denoise(space, stats::structural_analysis::dbscan_options(radius, min_points), runtime_policy);
}

} // namespace mtrc::modify::resample

namespace mtrc {
using modify::resample::denoise;
} // namespace mtrc

#endif
