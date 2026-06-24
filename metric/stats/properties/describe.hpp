// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_PROPERTIES_DESCRIBE_HPP
#define _METRIC_STATS_PROPERTIES_DESCRIBE_HPP

#include <cstddef>
#include <type_traits>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/space/storage/distance_matrix.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>

namespace mtrc::stats::properties {

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto describe_structure(const Provider &provider) -> StructureDescription<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	const auto distances = space::storage::provider_symmetric_distance_matrix(provider);
	core::ScalarAccumulator<distance_type> distance_summary;
	std::size_t pair_count = 0;
	std::size_t zero_distance_pair_count = 0;
	distance_type minimum_nonzero_distance{};
	bool has_nonzero_distances = false;

	for (std::size_t row = 0; row < provider.record_count(); ++row) {
		for (std::size_t column = row + 1; column < provider.record_count(); ++column) {
			const auto distance = distances(row, column);
			++pair_count;
			distance_summary.add(distance);

			if (distance <= distance_type{}) {
				++zero_distance_pair_count;
			} else if (!has_nonzero_distances || distance < minimum_nonzero_distance) {
				minimum_nonzero_distance = distance;
				has_nonzero_distances = true;
			}
		}
	}

	return core::make_structure_description(
		provider.record_count(), pair_count, zero_distance_pair_count, minimum_nonzero_distance,
		distance_summary.maximum_or(), distance_summary.average_or(),
		core::expansion_dimension(distances, provider.record_count(), distance_type{}), has_nonzero_distances,
		"pairwise_distances");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto describe_structure(const Space &space) -> StructureDescription<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	auto result = describe_structure(provider);
	result.representation = "metric_space";
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto describe_structure(const Container &records, const Metric &metric)
	-> StructureDescription<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = describe_structure(space);
	result.representation = "records";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto describe_structure(const Space &space, space::storage::policy runtime_policy)
	-> StructureDescription<typename Space::distance_type>
{
	space::storage::require_exact_describe(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		space::storage::DistanceTable<Space> matrix(space);
		auto result = describe_structure(matrix);
		result.representation = space::storage::describe_representation(runtime_policy);
		return result;
	}

	auto result = describe_structure(space);
	result.representation = space::storage::describe_representation(runtime_policy);
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto describe_structure(const Container &records, const Metric &metric, space::storage::policy runtime_policy)
	-> StructureDescription<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	return describe_structure(space, runtime_policy);
}

} // namespace mtrc::stats::properties

namespace mtrc {
using stats::properties::describe_structure;
} // namespace mtrc

#endif
