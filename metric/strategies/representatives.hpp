// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STRATEGIES_REPRESENTATIVES_HPP
#define _METRIC_STRATEGIES_REPRESENTATIVES_HPP

#include <cstddef>

namespace metric::strategies {

struct farthest_first {
	farthest_first() = default;

	explicit farthest_first(std::size_t seed_index)
		: seed_index(seed_index)
	{
	}

	std::size_t seed_index{};
};

} // namespace metric::strategies

#endif
