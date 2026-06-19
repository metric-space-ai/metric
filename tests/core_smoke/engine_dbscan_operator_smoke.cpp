// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		const auto difference = lhs - rhs;
		return difference < 0 ? -difference : difference;
	}
};

int main()
{
	auto space = metric::make_space(std::vector<int>{0, 1, 10, 11, 30}, AbsoluteDistance{});
	metric::representations::MatrixCache<decltype(space)> matrix(space);

	const auto clustered = metric::operators::dbscan(matrix, 2, 2);
	using Result = decltype(clustered);
	static_assert(std::is_same<typename Result::distance_type, int>::value);

	assert(clustered.algorithm == "dbscan");
	assert(clustered.representation == "distance_provider");
	assert(clustered.record_count == space.size());
	assert(clustered.cluster_count == 2);
	assert(clustered.noise_count == 1);
	assert(clustered.iterations == 1);
	assert(clustered.converged);
	assert(clustered.assignments.size() == space.size());
	assert(clustered.assignments[0] == 0);
	assert(clustered.assignments[1] == 0);
	assert(clustered.assignments[2] == 1);
	assert(clustered.assignments[3] == 1);
	assert(clustered.assignments[4] == Result::noise_label);
	assert(clustered.cluster_sizes[0] == 2);
	assert(clustered.cluster_sizes[1] == 2);
	assert(clustered.medoids[0] == space.id(0));
	assert(clustered.medoids[1] == space.id(2));
	assert(clustered.core_records.size() == 4);
	assert(clustered.core_records[0] == space.id(0));
	assert(clustered.core_records[3] == space.id(3));
	assert(clustered.noise_records.size() == 1);
	assert(clustered.noise_records[0] == space.id(4));

	const auto implicit = metric::operators::dbscan(space, 2, 2);
	assert(implicit.representation == "metric_space");
	assert(implicit.assignments == clustered.assignments);
	assert(implicit.noise_records == clustered.noise_records);
	assert(implicit.medoids == clustered.medoids);

	bool rejected_bad_radius = false;
	try {
		(void)metric::operators::dbscan(matrix, -1, 2);
	} catch (const std::invalid_argument &) {
		rejected_bad_radius = true;
	}
	assert(rejected_bad_radius);

	bool rejected_bad_min_points = false;
	try {
		(void)metric::operators::dbscan(matrix, 1, 0);
	} catch (const std::invalid_argument &) {
		rejected_bad_min_points = true;
	}
	assert(rejected_bad_min_points);

	return 0;
}
