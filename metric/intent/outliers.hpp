// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_OUTLIERS_HPP
#define _METRIC_INTENT_OUTLIERS_HPP

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <vector>

#include "../core/concepts.hpp"
#include "../core/result.hpp"
#include "../operators/clustering.hpp"
#include "../representations/implicit.hpp"
#include "../representations/matrix_cache.hpp"
#include "../runtime/execution.hpp"
#include "../strategies/clustering.hpp"

namespace metric::intent {
namespace detail {

template <typename Provider>
auto nearest_reference_distance(const Provider &provider, RecordId id, const std::vector<RecordId> &references)
	-> typename Provider::distance_type
{
	using distance_type = typename Provider::distance_type;

	distance_type best{};
	bool has_best = false;
	if (!references.empty()) {
		for (const auto reference : references) {
			const auto distance = provider.distance(id, reference);
			if (!has_best || distance < best) {
				best = distance;
				has_best = true;
			}
		}
		return best;
	}

	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		const auto reference = RecordId::from_index(index);
		if (reference == id) {
			continue;
		}
		const auto distance = provider.distance(id, reference);
		if (!has_best || distance < best) {
			best = distance;
			has_best = true;
		}
	}

	return best;
}

template <typename Provider>
auto outlier_result_from_groups(const Provider &provider,
								const ClusteringResult<typename Provider::distance_type> &groups)
	-> OutlierResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	const auto noise_label = ClusteringResult<distance_type>::noise_label;

	std::vector<RecordId> references;
	references.reserve(provider.record_count() - groups.noise_count);
	for (std::size_t index = 0; index < groups.assignments.size(); ++index) {
		if (groups.assignments[index] != noise_label) {
			references.push_back(RecordId::from_index(index));
		}
	}

	OutlierResult<distance_type> result;
	result.record_count = groups.record_count;
	result.cluster_count = groups.cluster_count;
	result.noise_count = groups.noise_count;
	result.exact = true;
	result.operator_name = "find_outliers";
	result.strategy = "dbscan_noise";
	result.representation = groups.representation;
	result.outliers.reserve(groups.noise_records.size());

	for (const auto id : groups.noise_records) {
		result.outliers.push_back({id, nearest_reference_distance(provider, id, references)});
	}

	std::sort(result.outliers.begin(), result.outliers.end(), [](const auto &lhs, const auto &rhs) {
		if (lhs.score == rhs.score) {
			return lhs.id.index() < rhs.id.index();
		}
		return lhs.score > rhs.score;
	});

	return result;
}

} // namespace detail

template <typename Provider, typename std::enable_if<DistanceProvider_v<Provider>, int>::type = 0>
auto find_outliers(const Provider &provider, strategies::dbscan strategy)
	-> OutlierResult<typename Provider::distance_type>
{
	const auto groups = operators::dbscan(provider, strategy.radius, strategy.min_points);
	return detail::outlier_result_from_groups(provider, groups);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, strategies::dbscan strategy) -> OutlierResult<typename Space::distance_type>
{
	representations::ImplicitDistanceProvider<Space> provider(space);
	auto result = find_outliers(provider, strategy);
	result.representation = "metric_space";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, strategies::dbscan strategy, runtime::policy runtime_policy)
	-> OutlierResult<typename Space::distance_type>
{
	runtime::require_exact_outliers(runtime_policy);
	runtime::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		representations::MatrixCache<Space> matrix(space);
		auto result = find_outliers(matrix, strategy);
		result.representation = runtime::outlier_representation(runtime_policy);
		return result;
	}

	auto result = find_outliers(space, strategy);
	result.representation = runtime::outlier_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, double radius, std::size_t min_points)
	-> OutlierResult<typename Space::distance_type>
{
	return find_outliers(space, strategies::dbscan(radius, min_points));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, double radius, std::size_t min_points, runtime::policy runtime_policy)
	-> OutlierResult<typename Space::distance_type>
{
	return find_outliers(space, strategies::dbscan(radius, min_points), runtime_policy);
}

} // namespace metric::intent

namespace metric {
using intent::find_outliers;
} // namespace metric

#endif
