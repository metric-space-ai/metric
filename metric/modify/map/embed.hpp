// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_EMBED_HPP
#define _METRIC_INTENT_EMBED_HPP

#include <cstddef>
#include <type_traits>

#include <metric/core/concepts.hpp>
#include <metric/modify/map/pcfa.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/modify/map/options.hpp>

namespace mtrc {

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto embed(const Space &space, modify::map::pcfa_options strategy) -> decltype(modify::map::pcfa_space(space, strategy.components))
{
	auto result = modify::map::pcfa_space(space, strategy.components);
	result.mapping = "pcfa_embedding";
	result.validity = "coordinate embedding; derived Euclidean coordinate space approximating source geometry; "
					  "not record compression";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto embed(const Space &space, modify::map::pcfa_options strategy, space::storage::policy runtime_policy)
	-> decltype(embed(space, strategy))
{
	space::storage::require_exact_embed(runtime_policy);
	space::storage::require_lazy_embed(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	auto result = embed(space, strategy);
	result.representation = space::storage::embed_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto embed(const Space &space, std::size_t components) -> decltype(embed(space, modify::map::pcfa_options(components)))
{
	return embed(space, modify::map::pcfa_options(components));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto embed(const Space &space, std::size_t components, space::storage::policy runtime_policy)
	-> decltype(embed(space, modify::map::pcfa_options(components), runtime_policy))
{
	return embed(space, modify::map::pcfa_options(components), runtime_policy);
}

} // namespace mtrc

#endif
