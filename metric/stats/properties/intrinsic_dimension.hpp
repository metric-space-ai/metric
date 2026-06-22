// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_PROPERTIES_INTRINSIC_DIMENSION_HPP
#define _METRIC_STATS_PROPERTIES_INTRINSIC_DIMENSION_HPP

#include <type_traits>
#include <utility>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/space/storage/distance_matrix.hpp>
#include <metric/space/storage/implicit.hpp>

namespace mtrc::stats::properties {

// Expansion dimension is a finite-space diagnostic: it estimates how quickly
// metric balls grow at the radii present in the observed record set.
template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto intrinsic_dimension(const Provider &provider) -> double
{
	using distance_type = typename Provider::distance_type;

	if (provider.record_count() < 2) {
		return 0.0;
	}

	const auto distances = space::storage::provider_symmetric_distance_matrix(provider);
	return core::expansion_dimension(distances, provider.record_count(), distance_type{});
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto intrinsic_dimension(const Space &space) -> double
{
	space::storage::LiveDistances<Space> provider(space);
	return intrinsic_dimension(provider);
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto intrinsic_dimension(const Container &records, Metric metric) -> double
{
	auto space = mtrc::make_space(records, std::move(metric));
	return intrinsic_dimension(space);
}

} // namespace mtrc::stats::properties

#endif
