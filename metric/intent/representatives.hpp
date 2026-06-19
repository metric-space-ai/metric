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
#include "../strategies/representatives.hpp"

namespace metric::intent {

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, std::size_t count, strategies::farthest_first strategy = {})
	-> RepresentativeSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;

	RepresentativeSet<distance_type> result;
	result.record_count = space.size();
	result.requested_count = count;
	result.exact = true;
	result.operator_name = "find_representatives";
	result.strategy = "farthest_first";
	result.representation = "metric_space";

	if (count == 0) {
		return result;
	}
	if (space.empty()) {
		throw std::invalid_argument("cannot select representatives from an empty metric space");
	}
	if (count > space.size()) {
		throw std::invalid_argument("representative count cannot exceed the number of records");
	}
	if (strategy.seed_index >= space.size()) {
		throw std::out_of_range("seed_index is outside the metric space");
	}

	result.representatives.push_back(space.id(strategy.seed_index));

	std::vector<bool> is_selected(space.size(), false);
	is_selected[strategy.seed_index] = true;

	result.nearest_representative_distances.resize(space.size());
	for (std::size_t index = 0; index < space.size(); ++index) {
		result.nearest_representative_distances[index] =
			space.distance(space.id(index), result.representatives.front());
	}

	while (result.representatives.size() < count) {
		std::size_t next_index = space.size();
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

		const auto next_id = space.id(next_index);
		result.representatives.push_back(next_id);
		is_selected[next_index] = true;
		for (std::size_t index = 0; index < result.nearest_representative_distances.size(); ++index) {
			result.nearest_representative_distances[index] =
				std::min(result.nearest_representative_distances[index], space.distance(space.id(index), next_id));
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

} // namespace metric::intent

namespace metric {
using intent::find_representatives;
} // namespace metric

#endif
