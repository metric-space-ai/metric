// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_COMPRESS_HPP
#define _METRIC_INTENT_COMPRESS_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "../core/concepts.hpp"
#include "../core/metric_space.hpp"
#include "../core/result.hpp"
#include "../representations/implicit.hpp"
#include "../representations/matrix_cache.hpp"
#include "../runtime/execution.hpp"
#include "../strategies/representatives.hpp"
#include "representatives.hpp"

namespace metric::intent {
namespace detail {

template <typename Space, typename Provider>
auto compress_from_representatives(const Space &space, const Provider &provider,
								   const RepresentativeSet<typename Space::distance_type> &representatives)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	using target_space_type = MetricSpace<typename Space::record_type, typename Space::metric_type>;
	using result_type = CompressionResult<target_space_type>;

	std::vector<typename Space::record_type> records;
	records.reserve(representatives.representatives.size());
	for (const auto id : representatives.representatives) {
		records.push_back(space.record(id));
	}

	std::vector<std::size_t> assignments;
	std::vector<typename Space::distance_type> nearest_distances;
	assignments.reserve(space.size());
	nearest_distances.reserve(space.size());

	for (std::size_t record_index = 0; record_index < space.size(); ++record_index) {
		std::size_t best_representative_index = 0;
		typename Space::distance_type best_distance{};
		bool has_best = false;
		const auto source_id = space.id(record_index);

		for (std::size_t representative_index = 0; representative_index < representatives.representatives.size();
			 ++representative_index) {
			const auto representative_id = representatives.representatives[representative_index];
			const auto distance = provider.distance(source_id, representative_id);
			if (!has_best || distance < best_distance ||
				(distance == best_distance &&
				 representative_id.index() < representatives.representatives[best_representative_index].index())) {
				best_representative_index = representative_index;
				best_distance = distance;
				has_best = true;
			}
		}

		assignments.push_back(best_representative_index);
		nearest_distances.push_back(best_distance);
	}

	const auto compressed_count = records.size();
	target_space_type compressed_space(std::move(records), space.metric());
	return result_type{std::move(compressed_space),
					   representatives.representatives,
					   std::move(assignments),
					   std::move(nearest_distances),
					   space.size(),
					   compressed_count,
					   static_cast<double>(compressed_count) / static_cast<double>(space.size()),
					   true,
					   "compress",
					   "representatives",
					   representatives.strategy,
					   representatives.representation,
					   true,
					   false};
}

} // namespace detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, std::size_t count, strategies::farthest_first strategy = {})
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	if (count == 0) {
		throw std::invalid_argument("compression count must be positive");
	}

	representations::ImplicitDistanceProvider<Space> provider(space);
	const auto representatives = find_representatives(space, count, strategy);
	return detail::compress_from_representatives(space, provider, representatives);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, std::size_t count, strategies::farthest_first strategy,
			  runtime::policy runtime_policy)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	if (count == 0) {
		throw std::invalid_argument("compression count must be positive");
	}

	runtime::require_exact_compress(runtime_policy);
	runtime::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		representations::MatrixCache<Space> matrix(space);
		auto representatives = find_representatives(matrix, count, strategy);
		representatives.representation = runtime::compression_representation(runtime_policy);
		return detail::compress_from_representatives(space, matrix, representatives);
	}

	auto result = compress(space, count, strategy);
	result.representation = runtime::compression_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, std::size_t count, runtime::policy runtime_policy)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	return compress(space, count, strategies::farthest_first{}, runtime_policy);
}

} // namespace metric::intent

namespace metric {
using intent::compress;
} // namespace metric

#endif
