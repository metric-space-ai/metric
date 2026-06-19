// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STRATEGIES_CLUSTERING_HPP
#define _METRIC_STRATEGIES_CLUSTERING_HPP

#include <cstddef>

namespace metric::strategies {

struct k_medoids {
	explicit k_medoids(std::size_t groups, std::size_t max_iterations = 100)
		: groups(groups)
		, max_iterations(max_iterations)
	{
	}

	std::size_t groups{};
	std::size_t max_iterations{100};
};

struct dbscan {
	dbscan(double radius, std::size_t min_points)
		: radius(radius)
		, min_points(min_points)
	{
	}

	double radius{};
	std::size_t min_points{};
};

} // namespace metric::strategies

#endif
