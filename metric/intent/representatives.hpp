// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_REPRESENTATIVES_HPP
#define _METRIC_INTENT_REPRESENTATIVES_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "../core/concepts.hpp"
#include "../core/result.hpp"
#include "../representations/implicit.hpp"
#include "../representations/matrix_cache.hpp"
#include "../runtime/execution.hpp"
#include "../strategies/representatives.hpp"

namespace metric::intent {

template <typename Provider, typename std::enable_if<DistanceProvider_v<Provider>, int>::type = 0>
auto find_representatives(const Provider &provider, std::size_t count, strategies::farthest_first strategy = {})
	-> RepresentativeSet<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	RepresentativeSet<distance_type> result;
	result.record_count = provider.record_count();
	result.requested_count = count;
	result.exact = true;
	result.operator_name = "find_representatives";
	result.strategy = "farthest_first";
	result.representation = "distance_provider";

	if (count == 0) {
		return result;
	}
	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot select representatives from an empty distance provider");
	}
	if (count > provider.record_count()) {
		throw std::invalid_argument("representative count cannot exceed the number of records");
	}
	if (strategy.seed_index >= provider.record_count()) {
		throw std::out_of_range("seed_index is outside the metric space");
	}

	result.representatives.push_back(RecordId::from_index(strategy.seed_index));

	std::vector<bool> is_selected(provider.record_count(), false);
	is_selected[strategy.seed_index] = true;

	result.nearest_representative_distances.resize(provider.record_count());
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		result.nearest_representative_distances[index] =
			provider.distance(RecordId::from_index(index), result.representatives.front());
	}

	while (result.representatives.size() < count) {
		std::size_t next_index = provider.record_count();
		distance_type next_distance{};
		bool has_next = false;

		for (std::size_t index = 0; index < result.nearest_representative_distances.size(); ++index) {
			if (is_selected[index]) {
				continue;
			}
			if (!has_next || result.nearest_representative_distances[index] > next_distance) {
				next_index = index;
				next_distance = result.nearest_representative_distances[index];
				has_next = true;
			}
		}

		if (!has_next) {
			throw std::logic_error("failed to select the next representative");
		}

		const auto next_id = RecordId::from_index(next_index);
		result.representatives.push_back(next_id);
		is_selected[next_index] = true;
		for (std::size_t index = 0; index < result.nearest_representative_distances.size(); ++index) {
			result.nearest_representative_distances[index] =
				std::min(result.nearest_representative_distances[index],
						 provider.distance(RecordId::from_index(index), next_id));
		}
	}

	double distance_sum = 0.0;
	for (const auto distance : result.nearest_representative_distances) {
		if (result.coverage_radius < distance) {
			result.coverage_radius = distance;
		}
		distance_sum += static_cast<double>(distance);
	}
	if (!result.nearest_representative_distances.empty()) {
		result.average_nearest_distance =
			distance_sum / static_cast<double>(result.nearest_representative_distances.size());
	}

	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, std::size_t count, strategies::farthest_first strategy = {})
	-> RepresentativeSet<typename Space::distance_type>
{
	representations::ImplicitDistanceProvider<Space> provider(space);
	auto result = find_representatives(provider, count, strategy);
	result.representation = "metric_space";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, std::size_t count, strategies::farthest_first strategy,
						  runtime::policy runtime_policy) -> RepresentativeSet<typename Space::distance_type>
{
	runtime::require_exact_representatives(runtime_policy);
	runtime::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		representations::MatrixCache<Space> matrix(space);
		auto result = find_representatives(matrix, count, strategy);
		result.representation = runtime::representative_representation(runtime_policy);
		return result;
	}

	auto result = find_representatives(space, count, strategy);
	result.representation = runtime::representative_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, std::size_t count, runtime::policy runtime_policy)
	-> RepresentativeSet<typename Space::distance_type>
{
	return find_representatives(space, count, strategies::farthest_first{}, runtime_policy);
}

} // namespace metric::intent

namespace metric {
using intent::find_representatives;
} // namespace metric

#endif
