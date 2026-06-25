// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_PROPERTIES_INTRINSIC_DIMENSION_HPP
#define _METRIC_STATS_PROPERTIES_INTRINSIC_DIMENSION_HPP

#include <type_traits>
#include <utility>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/stats/properties/describe.hpp>
#include <metric/space/storage/distance_matrix.hpp>
#include <metric/space/storage/implicit.hpp>

namespace mtrc::stats::properties {

// Expansion dimension is a finite-space diagnostic: it estimates how quickly
// metric balls grow at the radii present in the observed record set.
template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto intrinsic_dimension(const Provider &provider, describe_options options = {}) -> double
{
	using distance_type = typename Provider::distance_type;

	if (provider.record_count() < 2) {
		return 0.0;
	}

	if (provider.record_count() > options.max_exact_intrinsic_records &&
		options.allow_approximate_intrinsic_dimension) {
		return describe_detail::sampled_expansion_dimension(provider, options);
	}

	const auto distances = space::storage::provider_symmetric_distance_matrix(provider);
	return core::expansion_dimension(distances, provider.record_count(), distance_type{});
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto intrinsic_dimension(const Space &space, describe_options options = {}) -> double
{
	space::storage::LiveDistances<Space> provider(space);
	return intrinsic_dimension(provider, std::move(options));
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto intrinsic_dimension(const Container &records, Metric metric, describe_options options = {}) -> double
{
	auto space = mtrc::make_space(records, std::move(metric));
	return intrinsic_dimension(space, std::move(options));
}

} // namespace mtrc::stats::properties

namespace mtrc {
// Re-export so intrinsic_dimension sits at the same `mtrc::` depth as the other
// space-level verbs.
using stats::properties::intrinsic_dimension;
} // namespace mtrc

#endif
