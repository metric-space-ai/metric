// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_REDUCE_HPP
#define _METRIC_INTENT_REDUCE_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "../core/concepts.hpp"
#include "../core/metric_space.hpp"
#include "../core/result.hpp"
#include "../distance/k-related/Standards.hpp"
#include "../mappings/pcfa.hpp"
#include "../runtime/execution.hpp"
#include "../strategies/reduction.hpp"

namespace metric {

namespace engine_reduce_detail {

template <typename Space>
using roadmap_reduction_result_t = MappingResult<MetricSpace<std::vector<double>, Euclidean<double>>>;

[[noreturn]] inline auto throw_unpromoted_reduction_strategy(const char *strategy) -> void
{
	throw std::invalid_argument(std::string(strategy) +
								" reduction strategy is not promoted yet; use metric::strategies::pcfa(...) for the "
								"promoted C++ reduction path");
}

} // namespace engine_reduce_detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto reduce(const Space &space, strategies::pcfa strategy)
	-> decltype(mappings::pcfa_space(space, strategy.components))
{
	return mappings::pcfa_space(space, strategy.components);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto reduce(const Space &space, strategies::pcfa strategy, runtime::policy runtime_policy)
	-> decltype(reduce(space, strategy))
{
	runtime::require_exact_reduce(runtime_policy);
	runtime::require_lazy_reduce(runtime_policy);
	runtime::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	auto result = reduce(space, strategy);
	result.representation = runtime::reduce_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto reduce(const Space &space, std::size_t components)
	-> decltype(reduce(space, strategies::pcfa(components)))
{
	return reduce(space, strategies::pcfa(components));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto reduce(const Space &space, std::size_t components, runtime::policy runtime_policy)
	-> decltype(reduce(space, strategies::pcfa(components), runtime_policy))
{
	return reduce(space, strategies::pcfa(components), runtime_policy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto reduce(const Space &, strategies::som) -> engine_reduce_detail::roadmap_reduction_result_t<Space>
{
	engine_reduce_detail::throw_unpromoted_reduction_strategy("SOM");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto reduce(const Space &space, strategies::som strategy, runtime::policy runtime_policy)
	-> decltype(reduce(space, strategy))
{
	runtime::require_exact_reduce(runtime_policy);
	runtime::require_lazy_reduce(runtime_policy);
	runtime::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	return reduce(space, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto reduce(const Space &, strategies::koc) -> engine_reduce_detail::roadmap_reduction_result_t<Space>
{
	engine_reduce_detail::throw_unpromoted_reduction_strategy("KOC");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto reduce(const Space &space, strategies::koc strategy, runtime::policy runtime_policy)
	-> decltype(reduce(space, strategy))
{
	runtime::require_exact_reduce(runtime_policy);
	runtime::require_lazy_reduce(runtime_policy);
	runtime::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	return reduce(space, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto reduce(const Space &, strategies::dspcc) -> engine_reduce_detail::roadmap_reduction_result_t<Space>
{
	engine_reduce_detail::throw_unpromoted_reduction_strategy("DSPCC");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto reduce(const Space &space, strategies::dspcc strategy, runtime::policy runtime_policy)
	-> decltype(reduce(space, strategy))
{
	runtime::require_exact_reduce(runtime_policy);
	runtime::require_lazy_reduce(runtime_policy);
	runtime::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	return reduce(space, strategy);
}

} // namespace metric

#endif
