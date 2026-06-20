// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STRATEGIES_REPRESENTATION_HPP
#define _METRIC_STRATEGIES_REPRESENTATION_HPP

#include "../representations/strategy.hpp"

namespace metric::strategies {

template <typename Space, typename Strategy>
auto make_representation(const Space &space, Strategy strategy) -> decltype(metric::representations::make(space, strategy))
{
	return metric::representations::make(space, strategy);
}

} // namespace metric::strategies

#endif
