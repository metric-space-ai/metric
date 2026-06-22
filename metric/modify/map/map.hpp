// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MODIFY_MAP_HPP
#define _METRIC_MODIFY_MAP_HPP

#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/record/id.hpp>
#include <metric/core/result.hpp>
#include <metric/modify/map/mapping.hpp>
#include <metric/space/storage/execution.hpp>

namespace mtrc::modify::map {

template <typename Space, typename Transform, typename TargetMetric,
		  typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto map(const Space &space, Transform transform, TargetMetric metric) -> MappingResult<MetricSpace<
	typename std::decay<decltype(std::declval<Transform>()(std::declval<const typename Space::record_type &>()))>::type,
	typename std::decay<TargetMetric>::type>>
{
	using target_record_type = typename std::decay<decltype(std::declval<Transform>()(
		std::declval<const typename Space::record_type &>()))>::type;
	using target_metric_type = typename std::decay<TargetMetric>::type;
	using target_space_type = MetricSpace<target_record_type, target_metric_type>;

	auto records = core::transformed_records(space, transform);
	auto lineage = ::mtrc::one_to_one_lineage(space);
	target_space_type derived_space(std::move(records), std::move(metric));
	// A deterministic per-record transform produces a new finite metric space
	// whose metric law is exactly the target metric's law, and the transform is
	// a pure function so it applies out-of-sample to any new record.
	return core::make_mapping_result(std::move(derived_space), std::move(lineage.source_records),
									 std::move(lineage.representative_records), space.size(), false,
									 "deterministic_transform", "deterministic_transform", "metric_space",
									 core::metric_traits<target_metric_type>::law, true,
									 "deterministic per-record transform; applicable out-of-sample");
}

template <typename Space, typename Transform, typename TargetMetric,
		  typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto map(const Space &space, Transform transform, TargetMetric metric, space::storage::policy runtime_policy)
	-> decltype(map(space, transform, metric))
{
	space::storage::require_exact_map(runtime_policy);
	space::storage::require_lazy_map(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	auto result = map(space, std::move(transform), std::move(metric));
	result.representation = space::storage::map_representation(runtime_policy);
	return result;
}

template <typename Space, typename Mapping,
		  typename std::enable_if<MetricSpaceLike_v<Space> && Mapping_v<Mapping, Space>, int>::type = 0>
auto map(const Space &space, const Mapping &mapping)
	-> decltype(modify::map::transform(modify::map::fit(mapping, space), space))
{
	auto model = modify::map::fit(mapping, space);
	return modify::map::transform(model, space);
}

template <typename Space, typename Mapping,
		  typename std::enable_if<MetricSpaceLike_v<Space> && Mapping_v<Mapping, Space>, int>::type = 0>
auto map(const Space &space, const Mapping &mapping, space::storage::policy runtime_policy) -> decltype(map(space, mapping))
{
	space::storage::require_exact_map(runtime_policy);
	space::storage::require_lazy_map(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	auto result = map(space, mapping);
	result.representation = space::storage::map_representation(runtime_policy);
	return result;
}

} // namespace mtrc::modify::map

namespace mtrc {
using modify::map::map;
} // namespace mtrc

#endif
