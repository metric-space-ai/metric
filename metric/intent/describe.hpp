// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_DESCRIBE_HPP
#define _METRIC_INTENT_DESCRIBE_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <type_traits>
#include <vector>

#include "../core/concepts.hpp"
#include "../core/result.hpp"
#include "../representations/implicit.hpp"
#include "../representations/matrix_cache.hpp"
#include "../runtime/execution.hpp"

namespace metric::intent {
namespace detail {

template <typename Distance>
auto expansion_dimension(const std::vector<std::vector<Distance>> &distances) -> double
{
	double maximum_dimension = 0.0;
	for (const auto &row : distances) {
		for (const auto &radius : row) {
			if (radius <= Distance{}) {
				continue;
			}

			const auto outer_radius = radius + radius;
			const auto inner_count = static_cast<double>(
				std::count_if(row.begin(), row.end(), [&](const auto &value) { return value <= radius; }));
			const auto outer_count = static_cast<double>(
				std::count_if(row.begin(), row.end(), [&](const auto &value) { return value <= outer_radius; }));

			if (inner_count > 0.0 && outer_count >= inner_count) {
				maximum_dimension = std::max(maximum_dimension, std::log(outer_count / inner_count) / std::log(2.0));
			}
		}
	}
	return maximum_dimension;
}

} // namespace detail

template <typename Provider, typename std::enable_if<DistanceProvider_v<Provider>, int>::type = 0>
auto describe_structure(const Provider &provider) -> StructureDescription<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	StructureDescription<distance_type> result;
	result.record_count = provider.record_count();
	result.exact = true;
	result.operator_name = "describe_structure";
	result.strategy = "exact_all_pairs";
	result.representation = "distance_provider";

	std::vector<std::vector<distance_type>> distances(provider.record_count(),
													  std::vector<distance_type>(provider.record_count()));
	double distance_sum = 0.0;

	for (std::size_t row = 0; row < provider.record_count(); ++row) {
		for (std::size_t column = row + 1; column < provider.record_count(); ++column) {
			const auto distance = provider.distance(RecordId::from_index(row), RecordId::from_index(column));
			distances[row][column] = distance;
			distances[column][row] = distance;
			++result.pair_count;
			distance_sum += static_cast<double>(distance);

			if (distance <= distance_type{}) {
				++result.zero_distance_pair_count;
			} else if (!result.has_nonzero_distances || distance < result.minimum_nonzero_distance) {
				result.minimum_nonzero_distance = distance;
				result.has_nonzero_distances = true;
			}

			if (result.pair_count == 1 || result.maximum_distance < distance) {
				result.maximum_distance = distance;
			}
		}
	}

	if (result.pair_count > 0) {
		result.average_distance = distance_sum / static_cast<double>(result.pair_count);
	}
	result.intrinsic_dimension = detail::expansion_dimension(distances);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto describe_structure(const Space &space) -> StructureDescription<typename Space::distance_type>
{
	representations::ImplicitDistanceProvider<Space> provider(space);
	auto result = describe_structure(provider);
	result.representation = "metric_space";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto describe_structure(const Space &space, runtime::policy runtime_policy)
	-> StructureDescription<typename Space::distance_type>
{
	runtime::require_exact_describe(runtime_policy);
	runtime::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		representations::MatrixCache<Space> matrix(space);
		auto result = describe_structure(matrix);
		result.representation = runtime::describe_representation(runtime_policy);
		return result;
	}

	auto result = describe_structure(space);
	result.representation = runtime::describe_representation(runtime_policy);
	return result;
}

} // namespace metric::intent

namespace metric {
using intent::describe_structure;
} // namespace metric

#endif
