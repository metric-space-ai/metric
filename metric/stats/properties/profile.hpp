// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_PROPERTIES_PROFILE_HPP
#define _METRIC_STATS_PROPERTIES_PROFILE_HPP

#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/stats/properties/describe.hpp>
#include <metric/stats/properties/distribution.hpp>
#include <metric/stats/properties/local_volume.hpp>

namespace mtrc::stats::properties {

// StatsProfile is the cohesive, practical diagnostic summary of an existing finite metric
// space: how many records, how many pairs, how many coincide (zero distance), the metric
// value range and average, and a finite-space growth diagnostic (intrinsic_dimension --
// an EXPANSION dimension at the observed radii, NOT a manifold-dimension guarantee). It is
// a read-only investigation: profile() never builds a metric and never derives a space.
//
// The core summary above is always computed in a single pairwise pass. The optional
// distance-distribution and local-volume sections are computed ONLY when requested through
// profile_options, so profile() has no hidden expensive behavior.
template <typename Distance> struct StatsProfile {
	using distance_type = Distance;

	std::size_t record_count{};
	std::size_t pair_count{};
	std::size_t zero_distance_pair_count{};
	Distance minimum_nonzero_distance{};
	Distance maximum_distance{};
	double average_distance{};
	double intrinsic_dimension{};

	// Status flags (cheap booleans derived from the counts above).
	bool is_empty{true};
	bool is_singleton{false};
	bool has_nonzero_distances{false};
	bool has_zero_distance_pairs{false};

	bool exact{true};
	std::string algorithm{"profile"};
	std::string representation;

	// Optional sections; the has_* flag is true only when the section was requested and
	// computed. When false the corresponding member is left default-constructed.
	bool has_distance_distribution{false};
	DistanceDistribution<Distance> distance_distribution;
	bool has_local_volume{false};
	LocalVolumeResult<Distance> local_volume;
};

struct profile_options {
	profile_options() = default;

	bool include_distance_distribution{false};
	distance_distribution_options distribution{};
	bool include_local_volume{false};
	double local_volume_radius{0.0};
};

namespace profile_detail {

template <typename Distance, typename Source>
auto fill_core_profile(StatsProfile<Distance> &profile, const StructureDescription<Distance> &structure,
					   const Source &source) -> void
{
	profile.record_count = structure.record_count;
	profile.pair_count = structure.pair_count;
	profile.zero_distance_pair_count = structure.zero_distance_pair_count;
	profile.minimum_nonzero_distance = structure.minimum_nonzero_distance;
	profile.maximum_distance = structure.maximum_distance;
	profile.average_distance = structure.average_distance;
	profile.intrinsic_dimension = structure.intrinsic_dimension;
	profile.has_nonzero_distances = structure.has_nonzero_distances;
	profile.has_zero_distance_pairs = structure.zero_distance_pair_count > 0;
	profile.is_empty = structure.record_count == 0;
	profile.is_singleton = structure.record_count == 1;
	(void)source;
}

} // namespace profile_detail

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto profile(const Provider &provider, profile_options options = {}) -> StatsProfile<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	StatsProfile<distance_type> result;
	result.representation = "pairwise_distances";
	const auto structure = describe_structure(provider);
	profile_detail::fill_core_profile(result, structure, provider);

	if (options.include_distance_distribution) {
		result.distance_distribution = distance_distribution(provider, options.distribution);
		result.has_distance_distribution = true;
	}
	if (options.include_local_volume) {
		result.local_volume = local_volume(provider, options.local_volume_radius);
		result.has_local_volume = true;
	}

	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto profile(const Space &space, profile_options options = {}) -> StatsProfile<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	auto result = profile(provider, std::move(options));
	result.representation = "metric_space";
	if (result.has_distance_distribution) {
		result.distance_distribution.representation = "metric_space";
	}
	if (result.has_local_volume) {
		result.local_volume.representation = "metric_space";
	}
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto profile(const Container &records, const Metric &metric, profile_options options = {})
	-> StatsProfile<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = profile(space, std::move(options));
	result.representation = "records";
	if (result.has_distance_distribution) {
		result.distance_distribution.representation = "records";
	}
	if (result.has_local_volume) {
		result.local_volume.representation = "records";
	}
	return result;
}

} // namespace mtrc::stats::properties

namespace mtrc::stats {
using properties::profile;
using properties::profile_options;
template <typename Distance> using StatsProfile = properties::StatsProfile<Distance>;
} // namespace mtrc::stats

namespace mtrc {
using stats::properties::profile;
using stats::properties::profile_options;
} // namespace mtrc

#endif
