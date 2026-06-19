// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_EMBED_HPP
#define _METRIC_INTENT_EMBED_HPP

#include <cstddef>
#include <type_traits>

#include "../core/concepts.hpp"
#include "../mappings/pcfa.hpp"
#include "../strategies/reduction.hpp"

namespace metric {

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto embed(const Space &space, strategies::pcfa strategy)
	-> decltype(mappings::pcfa_space(space, strategy.components))
{
	auto result = mappings::pcfa_space(space, strategy.components);
	result.mapping = "pcfa_embedding";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto embed(const Space &space, std::size_t components)
	-> decltype(embed(space, strategies::pcfa(components)))
{
	return embed(space, strategies::pcfa(components));
}

} // namespace metric

#endif
