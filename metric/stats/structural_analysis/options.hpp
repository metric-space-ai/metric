// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STRATEGIES_CLUSTERING_HPP
#define _METRIC_STRATEGIES_CLUSTERING_HPP

#include <cstddef>

namespace mtrc::stats::structural_analysis {

struct k_medoids_options {
	explicit k_medoids_options(std::size_t groups, std::size_t max_iterations = 100)
		: groups(groups), max_iterations(max_iterations)
	{
	}

	std::size_t groups{};
	std::size_t max_iterations{100};
};

struct dbscan_options {
	dbscan_options(double radius, std::size_t min_points) : radius(radius), min_points(min_points) {}

	double radius{};
	std::size_t min_points{};
};

struct affinity_propagation_options {
	explicit affinity_propagation_options(double preference = 0.5, int max_iterations = 200, double tolerance = 1.0e-6,
										  double damping = 0.5)
		: preference(preference), max_iterations(max_iterations), tolerance(tolerance), damping(damping)
	{
	}

	double preference{0.5};
	int max_iterations{200};
	double tolerance{1.0e-6};
	double damping{0.5};
};

} // namespace mtrc::stats::structural_analysis

namespace mtrc {
// Re-export the clustering option structs alongside the verbs that consume them
// (find_groups / find_outliers), so a call reads
// `mtrc::find_groups(space, mtrc::dbscan_options(0.7, 2))` instead of spelling the
// deeply-nested mtrc::stats::structural_analysis::dbscan_options.
using stats::structural_analysis::affinity_propagation_options;
using stats::structural_analysis::dbscan_options;
using stats::structural_analysis::k_medoids_options;
} // namespace mtrc

#endif
