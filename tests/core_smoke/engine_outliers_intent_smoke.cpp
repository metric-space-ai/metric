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

	const auto outliers = metric::find_outliers(space, metric::strategies::dbscan(2.0, 2));
	using Result = decltype(outliers);
	static_assert(std::is_same<typename Result::score_type, int>::value);

	assert(outliers.operator_name == "find_outliers");
	assert(outliers.strategy == "dbscan_noise");
	assert(outliers.representation == "metric_space");
	assert(outliers.record_count == space.size());
	assert(outliers.cluster_count == 2);
	assert(outliers.noise_count == 1);
	assert(outliers.size() == 1);
	assert(outliers[0].id == space.id(4));
	assert(outliers[0].score == 19);

	const auto direct = metric::find_outliers(space, 2.0, 2);
	assert(direct.size() == outliers.size());
	assert(direct[0].id == outliers[0].id);
	assert(direct[0].score == outliers[0].score);

	const auto dense = metric::find_outliers(space, metric::strategies::dbscan(100.0, 2));
	assert(dense.empty());
	assert(dense.noise_count == 0);

	bool rejected_empty_space = false;
	try {
		auto empty_space = metric::make_space(std::vector<int>{}, AbsoluteDistance{});
		(void)metric::find_outliers(empty_space, metric::strategies::dbscan(2.0, 2));
	} catch (const std::invalid_argument &) {
		rejected_empty_space = true;
	}
	assert(rejected_empty_space);

	return 0;
}
