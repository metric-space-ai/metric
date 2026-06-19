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
#include "../strategies/clustering.hpp"

namespace metric::intent {
namespace detail {

template <typename Space>
auto nearest_reference_distance(const Space &space, RecordId id, const std::vector<RecordId> &references)
	-> typename Space::distance_type
{
	using distance_type = typename Space::distance_type;

	distance_type best{};
	bool has_best = false;
	if (!references.empty()) {
		for (const auto reference : references) {
			const auto distance = space.distance(id, reference);
			if (!has_best || distance < best) {
				best = distance;
				has_best = true;
			}
		}
		return best;
	}

	for (std::size_t index = 0; index < space.size(); ++index) {
		const auto reference = space.id(index);
		if (reference == id) {
			continue;
		}
		const auto distance = space.distance(id, reference);
		if (!has_best || distance < best) {
			best = distance;
			has_best = true;
		}
	}

	return best;
}

} // namespace detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, strategies::dbscan strategy) -> OutlierResult<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;
	const auto groups = operators::dbscan(space, strategy.radius, strategy.min_points);
	const auto noise_label = ClusteringResult<distance_type>::noise_label;

	std::vector<RecordId> references;
	references.reserve(space.size() - groups.noise_count);
	for (std::size_t index = 0; index < groups.assignments.size(); ++index) {
		if (groups.assignments[index] != noise_label) {
			references.push_back(space.id(index));
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
		result.outliers.push_back({id, detail::nearest_reference_distance(space, id, references)});
	}

	std::sort(result.outliers.begin(), result.outliers.end(), [](const auto &lhs, const auto &rhs) {
		if (lhs.score == rhs.score) {
			return lhs.id.index() < rhs.id.index();
		}
		return lhs.score > rhs.score;
	});

	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, double radius, std::size_t min_points)
	-> OutlierResult<typename Space::distance_type>
{
	return find_outliers(space, strategies::dbscan(radius, min_points));
}

} // namespace metric::intent

namespace metric {
using intent::find_outliers;
} // namespace metric

#endif
