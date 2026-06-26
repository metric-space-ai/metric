// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_SAMPLE_OPTIONS_HPP
#define _METRIC_STATS_SAMPLE_OPTIONS_HPP

#include <cstddef>

namespace mtrc::stats::sample {

inline constexpr std::size_t default_sample_max_distance_evaluations = 100'000'000;

struct farthest_first_options {
	farthest_first_options() = default;
	explicit farthest_first_options(std::size_t seed_index) : seed_index(seed_index) {}

	std::size_t seed_index{};
	std::size_t max_distance_evaluations{default_sample_max_distance_evaluations};
};

struct metric_walk_options {
	metric_walk_options() = default;
	explicit metric_walk_options(std::size_t seed_index) : seed_index(seed_index) {}

	std::size_t seed_index{};
	std::size_t max_distance_evaluations{default_sample_max_distance_evaluations};
};

} // namespace mtrc::stats::sample

#endif
