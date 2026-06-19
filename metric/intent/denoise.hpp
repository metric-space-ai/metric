// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_DENOISE_HPP
#define _METRIC_INTENT_DENOISE_HPP

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include "../core/concepts.hpp"
#include "../core/metric_space.hpp"
#include "../core/record_id.hpp"
#include "../core/result.hpp"
#include "../operators/clustering.hpp"
#include "../representations/matrix_cache.hpp"
#include "../runtime/execution.hpp"
#include "../strategies/clustering.hpp"

namespace metric::intent {
namespace detail {

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto denoise_from_groups(const Space &space, const ClusteringResult<typename Space::distance_type> &groups)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	using target_space_type = MetricSpace<typename Space::record_type, typename Space::metric_type>;
	using result_type = MappingResult<target_space_type>;

	using clustering_type = typename std::decay<decltype(groups)>::type;
	std::vector<typename Space::record_type> records;
	std::vector<std::vector<RecordId>> source_records;
	std::vector<RecordId> representative_records;
	records.reserve(space.size());
	source_records.reserve(space.size());
	representative_records.reserve(space.size());

	for (std::size_t index = 0; index < groups.assignments.size(); ++index) {
		if (groups.assignments[index] == clustering_type::noise_label) {
			continue;
		}

		const auto id = space.id(index);
		records.push_back(space.record(id));
		source_records.push_back({id});
		representative_records.push_back(id);
	}

	target_space_type derived_space(std::move(records), space.metric());
	return result_type{std::move(derived_space), std::move(source_records), std::move(representative_records),
					   space.size(), false, "density_denoise", "dbscan_noise_filter", groups.representation};
}

} // namespace detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto denoise(const Space &space, strategies::dbscan strategy)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	const auto groups = operators::dbscan(space, strategy.radius, strategy.min_points);
	return detail::denoise_from_groups(space, groups);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto denoise(const Space &space, strategies::dbscan strategy, runtime::policy runtime_policy)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	runtime::require_exact_denoise(runtime_policy);
	runtime::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		representations::MatrixCache<Space> matrix(space);
		auto groups = operators::dbscan(matrix, strategy.radius, strategy.min_points);
		groups.representation = runtime::denoise_representation(runtime_policy);
		return detail::denoise_from_groups(space, groups);
	}

	auto result = denoise(space, strategy);
	result.representation = runtime::denoise_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto denoise(const Space &space, double radius, std::size_t min_points) -> decltype(denoise(
	space, strategies::dbscan(radius, min_points)))
{
	return denoise(space, strategies::dbscan(radius, min_points));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto denoise(const Space &space, double radius, std::size_t min_points, runtime::policy runtime_policy) -> decltype(
	denoise(space, strategies::dbscan(radius, min_points), runtime_policy))
{
	return denoise(space, strategies::dbscan(radius, min_points), runtime_policy);
}

} // namespace metric::intent

namespace metric {
using intent::denoise;
} // namespace metric

#endif
