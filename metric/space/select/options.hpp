// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STRATEGIES_REPRESENTATIVES_HPP
#define _METRIC_STRATEGIES_REPRESENTATIVES_HPP

#include <cstddef>

namespace mtrc::space::select {

struct farthest_first {
	farthest_first() = default;

	explicit farthest_first(std::size_t seed_index) : seed_index(seed_index) {}

	std::size_t seed_index{};
};

struct coverage {
	coverage() = default;

	explicit coverage(std::size_t seed_index) : seed_index(seed_index) {}

	std::size_t seed_index{};
};

struct k_center {
	k_center() = default;

	explicit k_center(std::size_t seed_index) : seed_index(seed_index) {}

	std::size_t seed_index{};
};

template <typename Radius> struct radius_coverage {
	explicit radius_coverage(Radius radius) : radius(radius) {}

	Radius radius;
};

template <typename Radius> radius_coverage(Radius) -> radius_coverage<Radius>;

} // namespace mtrc::space::select

#endif
