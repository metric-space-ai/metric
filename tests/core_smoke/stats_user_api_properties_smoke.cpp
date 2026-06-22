// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// User-facing stats properties: the cohesive profile() summary, distance-value
// distribution diagnostics, and the multi-radius local-volume profile. All are read-only
// investigations of an existing finite metric space; none builds a metric or a derived
// space. Vector records are used only as a convenient special case.

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/stats/properties/distribution.hpp"
#include "metric/stats/properties/intrinsic_dimension.hpp"
#include "metric/stats/properties/local_volume.hpp"
#include "metric/stats/properties/profile.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct NaNPairDistance {
	auto operator()(int lhs, int rhs) const -> double
	{
		if (lhs == rhs) {
			return 0.0;
		}
		return std::numeric_limits<double>::quiet_NaN();
	}
};

auto close(double lhs, double rhs, double tolerance = 1.0e-9) -> bool { return std::abs(lhs - rhs) < tolerance; }

template <typename Function> auto throws_invalid_argument(Function run) -> bool
{
	try {
		run();
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

void profile_summarizes_edge_spaces()
{
	namespace properties = mtrc::stats::properties;

	const auto empty_space = mtrc::make_space(std::vector<int>{}, AbsoluteDistance{});
	const auto empty_profile = properties::profile(empty_space);
	assert(empty_profile.record_count == 0);
	assert(empty_profile.pair_count == 0);
	assert(empty_profile.is_empty);
	assert(!empty_profile.has_nonzero_distances);
	assert(!empty_profile.has_distance_distribution);
	assert(!empty_profile.has_local_volume);
	assert(empty_profile.representation == "metric_space");

	const auto single_profile = properties::profile(mtrc::make_space(std::vector<int>{7}, AbsoluteDistance{}));
	assert(single_profile.record_count == 1);
	assert(single_profile.is_singleton);
	assert(single_profile.pair_count == 0);
	assert(!single_profile.has_nonzero_distances);

	const auto duplicate_profile =
		properties::profile(mtrc::make_space(std::vector<int>{3, 3, 3, 3}, AbsoluteDistance{}));
	assert(duplicate_profile.record_count == 4);
	assert(duplicate_profile.pair_count == 6);
	assert(duplicate_profile.zero_distance_pair_count == 6);
	assert(duplicate_profile.has_zero_distance_pairs);
	assert(!duplicate_profile.has_nonzero_distances);
	assert(close(duplicate_profile.intrinsic_dimension, 0.0));
}

void profile_optional_sections_only_when_requested()
{
	namespace properties = mtrc::stats::properties;
	const auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 10}, AbsoluteDistance{});

	const auto bare = properties::profile(space);
	assert(!bare.has_distance_distribution && !bare.has_local_volume);
	assert(bare.pair_count == 6);
	assert(bare.has_nonzero_distances);
	assert(bare.minimum_nonzero_distance == 1);
	assert(bare.maximum_distance == 10);

	properties::profile_options options;
	options.include_distance_distribution = true;
	options.include_local_volume = true;
	options.local_volume_radius = 2.0;
	const auto full = properties::profile(space, options);
	assert(full.has_distance_distribution);
	assert(full.has_local_volume);
	assert(full.distance_distribution.pair_count == 6);
	assert(full.distance_distribution.representation == "metric_space");
	assert(full.local_volume.record_count == 4);
	assert(full.local_volume.representation == "metric_space");
}

void distance_distribution_quantiles_and_histogram()
{
	namespace properties = mtrc::stats::properties;

	// Pairwise distances of {0,2,4,6} are {2,2,2,4,4,6} once sorted.
	const auto space = mtrc::make_space(std::vector<int>{0, 2, 4, 6}, AbsoluteDistance{});
	const auto distribution = properties::distance_distribution(space);
	assert(distribution.pair_count == 6);
	assert(distribution.minimum == 2);
	assert(distribution.maximum == 6);
	assert(close(distribution.median, 3.0));
	assert(close(distribution.mean, 20.0 / 6.0));
	assert(distribution.has_nonzero_distances);
	assert(distribution.zero_distance_pair_count == 0);

	// Default quantile probabilities {0, .25, .5, .75, 1} -> values {2,2,3,4,6}.
	assert(distribution.quantile_values.size() == 5);
	assert(distribution.quantile_values[0] == 2);
	assert(distribution.quantile_values[2] == 3);
	assert(distribution.quantile_values[4] == 6);

	// Histogram counts cover every pair exactly once.
	std::size_t histogram_total = 0;
	for (const auto count : distribution.histogram_counts) {
		histogram_total += count;
	}
	assert(histogram_total == distribution.pair_count);
	assert(distribution.histogram_edges.size() == distribution.histogram_counts.size() + 1);

	// Non-finite distances are rejected (a NaN has no defined order for sorting/quantiles).
	const auto nan_space = mtrc::make_space(std::vector<int>{0, 1, 2, 3}, NaNPairDistance{});
	assert(throws_invalid_argument([&] { (void)properties::distance_distribution(nan_space); }));
}

void local_volume_profile_traces_growth()
{
	namespace properties = mtrc::stats::properties;

	const auto space = mtrc::make_space(std::vector<int>{0, 0, 0, 1, 100}, AbsoluteDistance{});
	const auto profile = properties::local_volume_profile(space, std::vector<int>{0, 1});
	assert(profile.size() == 2);
	assert(profile.record_count == 5);
	assert(profile.representation == "metric_space");
	assert(profile.entries[0].radius == 0);
	assert(close(profile.entries[0].average_count, 2.2)); // counts {3,3,3,1,1}
	assert(profile.entries[1].radius == 1);
	assert(close(profile.entries[1].average_count, 17.0 / 5.0)); // counts {4,4,4,4,1}
	assert(profile.entries[0].minimum_count == 1);
	assert(profile.entries[0].maximum_count == 3);
}

void intrinsic_dimension_edges()
{
	namespace properties = mtrc::stats::properties;
	assert(close(properties::intrinsic_dimension(mtrc::make_space(std::vector<int>{}, AbsoluteDistance{})), 0.0));
	assert(close(properties::intrinsic_dimension(mtrc::make_space(std::vector<int>{5}, AbsoluteDistance{})), 0.0));
}

} // namespace

int main()
{
	profile_summarizes_edge_spaces();
	profile_optional_sections_only_when_requested();
	distance_distribution_quantiles_and_histogram();
	local_volume_profile_traces_growth();
	intrinsic_dimension_edges();
	return 0;
}
