// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// User-facing stats properties: the cohesive profile() summary, distance-value
// distribution diagnostics, and the multi-radius local-volume profile. All are read-only
// investigations of an existing finite metric space; none builds a metric or a derived
// space. Vector records are used only as a convenient special case.

#include <cassert>
#include <cstddef>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
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

struct CountingAbsoluteDistance {
	std::shared_ptr<std::size_t> calls;

	explicit CountingAbsoluteDistance(std::shared_ptr<std::size_t> call_counter)
		: calls(std::move(call_counter))
	{
	}

	auto operator()(int lhs, int rhs) const -> int
	{
		++(*calls);
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
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
	assert(full.local_volume.exact);
}

void distance_distribution_quantiles_and_histogram()
{
	namespace properties = mtrc::stats::properties;

	// Pairwise distances of {0,2,4,6} are {2,2,2,4,4,6} once sorted.
	const auto space = mtrc::make_space(std::vector<int>{0, 2, 4, 6}, AbsoluteDistance{});
	const auto distribution = properties::distance_distribution(space);
	assert(distribution.exact);
	assert(distribution.pair_count == 6);
	assert(distribution.evaluated_pair_count == 6);
	assert(distribution.sample_count == 6);
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
	assert(histogram_total == distribution.evaluated_pair_count);
	assert(distribution.histogram_edges.size() == distribution.histogram_counts.size() + 1);

	// Non-finite distances are rejected (a NaN has no defined order for sorting/quantiles).
	const auto nan_space = mtrc::make_space(std::vector<int>{0, 1, 2, 3}, NaNPairDistance{});
	assert(throws_invalid_argument([&] { (void)properties::distance_distribution(nan_space); }));
}

void distance_distribution_samples_large_spaces()
{
	namespace properties = mtrc::stats::properties;

	const auto small_space = mtrc::make_space(std::vector<int>{0, 1, 2, 3}, AbsoluteDistance{});
	const auto exact_distribution = properties::distance_distribution(small_space);
	assert(exact_distribution.exact);
	assert(exact_distribution.pair_count == 6);
	assert(exact_distribution.evaluated_pair_count == exact_distribution.pair_count);
	assert(exact_distribution.sample_count == exact_distribution.pair_count);

	std::vector<int> records;
	records.reserve(5000);
	for (int value = 0; value < 5000; ++value) {
		records.push_back(value);
	}
	const auto calls = std::make_shared<std::size_t>(0);
	const auto large_space = mtrc::make_space(records, CountingAbsoluteDistance(calls));
	const auto sampled_distribution = properties::distance_distribution(large_space);
	const auto default_options = properties::distance_distribution_options{};
	const auto expected_pair_count = records.size() * (records.size() - 1) / 2;

	assert(!sampled_distribution.exact);
	assert(sampled_distribution.pair_count == expected_pair_count);
	assert(sampled_distribution.evaluated_pair_count == default_options.sample_count);
	assert(sampled_distribution.sample_count == sampled_distribution.evaluated_pair_count);
	assert(*calls == sampled_distribution.evaluated_pair_count);
	assert(sampled_distribution.approximation_quality.diagnostic == "distance_distribution_approximation");
	assert(sampled_distribution.approximation_quality.candidate_policy == "regular_pair_ordinal_sample");
	assert(sampled_distribution.approximation_quality.candidate_count == sampled_distribution.evaluated_pair_count);
	assert(sampled_distribution.approximation_quality.candidate_universe == sampled_distribution.pair_count);
	assert(sampled_distribution.approximation_quality.distance_evaluations ==
		   sampled_distribution.evaluated_pair_count);
	assert(sampled_distribution.approximation_quality.sample_count == sampled_distribution.evaluated_pair_count);
	assert(sampled_distribution.approximation_quality.sample_universe == sampled_distribution.pair_count);
	assert(sampled_distribution.approximation_quality.sample_fraction > 0.0);
	assert(sampled_distribution.approximation_quality.sample_fraction < 0.001);
	assert(sampled_distribution.approximation_quality.standard_error > 0.0);
	assert(sampled_distribution.approximation_quality.confidence_radius_95 >
		   sampled_distribution.approximation_quality.standard_error);

	std::size_t histogram_total = 0;
	for (const auto count : sampled_distribution.histogram_counts) {
		histogram_total += count;
	}
	assert(histogram_total == sampled_distribution.evaluated_pair_count);

	properties::distance_distribution_options sampled_options;
	sampled_options.max_exact_pair_records = 1;
	sampled_options.sample_count = 2;
	const auto nan_space = mtrc::make_space(std::vector<int>{0, 1, 2, 3}, NaNPairDistance{});
	assert(throws_invalid_argument([&] { (void)properties::distance_distribution(nan_space, sampled_options); }));
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

void local_volume_samples_large_spaces()
{
	namespace properties = mtrc::stats::properties;

	std::vector<int> records;
	records.reserve(5000);
	for (int value = 0; value < 5000; ++value) {
		records.push_back(value);
	}

	const auto calls = std::make_shared<std::size_t>(0);
	const auto large_space = mtrc::make_space(records, CountingAbsoluteDistance(calls));
	const auto sampled_volume = properties::local_volume(large_space, 1);
	const auto default_options = properties::local_volume_options{};
	const auto exact_call_count = records.size() * records.size();
	const auto expected_sample_calls = records.size() * default_options.sample_count;

	assert(!sampled_volume.exact);
	assert(sampled_volume.algorithm == "sampled_local_volume");
	assert(sampled_volume.representation == "sampled_metric_space");
	assert(sampled_volume.record_count == records.size());
	assert(sampled_volume.size() == records.size());
	assert(sampled_volume.evaluated_distance_count == expected_sample_calls);
	assert(sampled_volume.sample_count == default_options.sample_count);
	assert(sampled_volume.sample_universe == records.size() - 1);
	assert(*calls == expected_sample_calls);
	assert(*calls < exact_call_count / 4);
	assert(sampled_volume.approximation_quality.diagnostic == "local_volume_approximation");
	assert(sampled_volume.approximation_quality.candidate_policy == "regular_target_sample");
	assert(sampled_volume.approximation_quality.candidate_count == default_options.sample_count);
	assert(sampled_volume.approximation_quality.candidate_universe == records.size() - 1);
	assert(sampled_volume.approximation_quality.distance_evaluations == expected_sample_calls);
	assert(sampled_volume.approximation_quality.sample_fraction > 0.0);
	assert(sampled_volume.approximation_quality.sample_fraction < 0.11);
	assert(!sampled_volume.approximation_quality.reason.empty());

	*calls = 0;
	properties::local_volume_options forced_sample_options;
	forced_sample_options.max_exact_records = 0;
	forced_sample_options.sample_count = 2;
	const auto forced_sampled_volume = properties::local_volume(large_space, 1, forced_sample_options);
	assert(!forced_sampled_volume.exact);
	assert(forced_sampled_volume.evaluated_distance_count == records.size() * forced_sample_options.sample_count);
	assert(*calls == forced_sampled_volume.evaluated_distance_count);

	*calls = 0;
	properties::profile_options profile_options;
	profile_options.include_local_volume = true;
	profile_options.local_volume_radius = 1.0;
	const auto profiled = properties::profile(large_space, profile_options);
	assert(profiled.has_local_volume);
	assert(!profiled.local_volume.exact);
	assert(profiled.local_volume.evaluated_distance_count == expected_sample_calls);
	const auto default_describe_options = properties::describe_options{};
	assert(*calls <= expected_sample_calls + default_describe_options.pair_summary_sample_count +
						 records.size() * default_describe_options.intrinsic_sample_count);
}

void intrinsic_dimension_edges()
{
	namespace properties = mtrc::stats::properties;
	assert(close(properties::intrinsic_dimension(mtrc::make_space(std::vector<int>{}, AbsoluteDistance{})), 0.0));
	assert(close(properties::intrinsic_dimension(mtrc::make_space(std::vector<int>{5}, AbsoluteDistance{})), 0.0));

	std::vector<int> records;
	records.reserve(300);
	for (int value = 0; value < 300; ++value) {
		records.push_back(value);
	}
	const auto calls = std::make_shared<std::size_t>(0);
	const auto large_space = mtrc::make_space(records, CountingAbsoluteDistance(calls));
	const auto sampled_dimension = properties::intrinsic_dimension(large_space);
	assert(sampled_dimension >= 0.0);
	assert(*calls <= records.size() * properties::describe_options{}.intrinsic_sample_count);
}

} // namespace

int main()
{
	profile_summarizes_edge_spaces();
	profile_optional_sections_only_when_requested();
	distance_distribution_quantiles_and_histogram();
	distance_distribution_samples_large_spaces();
	local_volume_profile_traces_growth();
	local_volume_samples_large_spaces();
	intrinsic_dimension_edges();
	return 0;
}
