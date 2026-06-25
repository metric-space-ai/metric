// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// mtrc::reduce here is COORDINATE (dimension) reduction: it maps records into a
// lower-dimensional derived coordinate space via a derived projection. It is a member of
// the modify::map family and the result carries metric_status/validity reporting
// that the derived space is a coordinate approximation, not a true-metric subset.
//
// This is deliberately distinct from mtrc::modify::reduce::compress (see
// metric/modify/reduce/compress.hpp), which is record-set CARDINALITY reduction
// (coverage-based coarsening over the same records). Dimension reduction is never
// labelled compression: see the validity strings populated by pcfa_space.

#ifndef _METRIC_MODIFY_REDUCE_HPP
#define _METRIC_MODIFY_REDUCE_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/metric/catalog/vector/Standards.hpp>
#include <metric/modify/map/pcfa.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/modify/map/options.hpp>

namespace mtrc {

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto reduce(const Space &space, modify::map::pcfa_options strategy) -> decltype(modify::map::pcfa_space(space, strategy.components))
{
	return modify::map::pcfa_space(space, strategy.components);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto reduce(const Space &space, modify::map::pcfa_options strategy, space::storage::policy runtime_policy)
	-> decltype(reduce(space, strategy))
{
	space::storage::require_exact_reduce(runtime_policy);
	space::storage::require_lazy_reduce(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	auto result = reduce(space, strategy);
	result.representation = space::storage::reduce_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto reduce(const Space &space, std::size_t components) -> decltype(reduce(space, modify::map::pcfa_options(components)))
{
	return reduce(space, modify::map::pcfa_options(components));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto reduce(const Space &space, std::size_t components, space::storage::policy runtime_policy)
	-> decltype(reduce(space, modify::map::pcfa_options(components), runtime_policy))
{
	return reduce(space, modify::map::pcfa_options(components), runtime_policy);
}

} // namespace mtrc

#endif
