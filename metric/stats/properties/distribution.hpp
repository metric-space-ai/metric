// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_PROPERTIES_DISTRIBUTION_HPP
#define _METRIC_STATS_PROPERTIES_DISTRIBUTION_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/space/storage/implicit.hpp>

namespace mtrc::stats::properties {

// distance_distribution summarizes the empirical distribution of the unordered pairwise
// metric values of a finite metric space (the i < j upper triangle). It is a diagnostic
// over an existing space: it never builds a metric or a derived space. Vector records are
// just one special case -- the diagnostic is computed from metric values, not coordinates.
//
// quantile_probabilities / quantile_values report sample quantiles via type-7 linear
// interpolation (between closest ranks) of the sorted pairwise values. histogram_edges /
// histogram_counts report a uniform-width histogram over [minimum, maximum] with
// bucket_count buckets (the last bucket is closed on the right so the maximum lands in it).
//
// Non-finite distances are REJECTED with std::invalid_argument: a NaN has no defined order,
// so sorting/quantiles would be undefined. A true metric always yields finite values.
template <typename Distance> struct DistanceDistribution {
	using distance_type = Distance;

	std::size_t record_count{};
	std::size_t pair_count{};
	std::size_t zero_distance_pair_count{};
	Distance minimum{};
	Distance maximum{};
	double mean{};
	double median{};
	std::vector<double> quantile_probabilities;
	std::vector<Distance> quantile_values;
	std::vector<Distance> histogram_edges;  // bucket_count + 1 edges (empty when no pairs)
	std::vector<std::size_t> histogram_counts; // bucket_count counts (empty when no pairs)
	bool has_nonzero_distances{false};
	bool exact{true};
	std::string algorithm{"distance_distribution"};
	std::string representation;

	auto empty() const -> bool { return pair_count == 0; }
	auto bucket_count() const -> std::size_t { return histogram_counts.size(); }
};

struct distance_distribution_options {
	distance_distribution_options() = default;

	std::size_t bucket_count{10};
	std::vector<double> quantile_probabilities{0.0, 0.25, 0.5, 0.75, 1.0};
};

namespace distribution_detail {

template <typename Value> auto distribution_finite_scalar(Value value) -> bool
{
	if constexpr (std::is_floating_point<Value>::value) {
		return std::isfinite(static_cast<double>(value));
	} else {
		(void)value;
		return true;
	}
}

// type-7 quantile (linear interpolation between closest ranks) over an ascending vector.
template <typename Distance>
auto interpolated_quantile(const std::vector<Distance> &sorted_values, double probability) -> Distance
{
	const auto count = sorted_values.size();
	if (count == 1) {
		return sorted_values.front();
	}
	const auto clamped = std::min(1.0, std::max(0.0, probability));
	const double position = clamped * static_cast<double>(count - 1);
	const auto lower_index = static_cast<std::size_t>(std::floor(position));
	const auto upper_index = static_cast<std::size_t>(std::ceil(position));
	if (lower_index == upper_index) {
		return sorted_values[lower_index];
	}
	const double fraction = position - static_cast<double>(lower_index);
	const double interpolated = static_cast<double>(sorted_values[lower_index]) * (1.0 - fraction) +
								static_cast<double>(sorted_values[upper_index]) * fraction;
	return static_cast<Distance>(interpolated);
}

} // namespace distribution_detail

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto distance_distribution(const Provider &provider, distance_distribution_options options = {})
	-> DistanceDistribution<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	DistanceDistribution<distance_type> result;
	result.record_count = provider.record_count();
	result.representation = "pairwise_distances";

	std::vector<distance_type> values;
	const auto record_count = provider.record_count();
	if (record_count >= 2) {
		values.reserve(record_count * (record_count - 1) / 2);
	}

	double sum = 0.0;
	for (std::size_t row = 0; row < record_count; ++row) {
		const auto row_id = provider.id(row);
		for (std::size_t column = row + 1; column < record_count; ++column) {
			const auto distance = provider.distance(row_id, provider.id(column));
			if (!distribution_detail::distribution_finite_scalar(distance)) {
				throw std::invalid_argument("distance_distribution requires finite distance values");
			}
			values.push_back(distance);
			sum += static_cast<double>(distance);
			if (distance <= distance_type{}) {
				++result.zero_distance_pair_count;
			} else {
				result.has_nonzero_distances = true;
			}
		}
	}

	result.pair_count = values.size();
	if (values.empty()) {
		return result;
	}

	std::sort(values.begin(), values.end());
	result.minimum = values.front();
	result.maximum = values.back();
	result.mean = sum / static_cast<double>(values.size());
	result.median = static_cast<double>(distribution_detail::interpolated_quantile(values, 0.5));

	result.quantile_probabilities = options.quantile_probabilities;
	result.quantile_values.reserve(result.quantile_probabilities.size());
	for (const auto probability : result.quantile_probabilities) {
		result.quantile_values.push_back(distribution_detail::interpolated_quantile(values, probability));
	}

	if (options.bucket_count > 0) {
		result.histogram_counts.assign(options.bucket_count, 0);
		result.histogram_edges.reserve(options.bucket_count + 1);
		const double minimum = static_cast<double>(result.minimum);
		const double maximum = static_cast<double>(result.maximum);
		const double span = maximum - minimum;
		for (std::size_t edge = 0; edge <= options.bucket_count; ++edge) {
			const double position = minimum + span * static_cast<double>(edge) / static_cast<double>(options.bucket_count);
			result.histogram_edges.push_back(static_cast<distance_type>(position));
		}
		for (const auto value : values) {
			std::size_t bucket = 0;
			if (span > 0.0) {
				const double normalized = (static_cast<double>(value) - minimum) / span;
				bucket = static_cast<std::size_t>(normalized * static_cast<double>(options.bucket_count));
				if (bucket >= options.bucket_count) {
					bucket = options.bucket_count - 1; // closed-right last bucket (the maximum)
				}
			}
			++result.histogram_counts[bucket];
		}
	}

	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto distance_distribution(const Space &space, distance_distribution_options options = {})
	-> DistanceDistribution<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	auto result = distance_distribution(provider, std::move(options));
	result.representation = "metric_space";
	return result;
}

} // namespace mtrc::stats::properties

namespace mtrc::stats {
using properties::distance_distribution;
using properties::distance_distribution_options;
template <typename Distance> using DistanceDistribution = properties::DistanceDistribution<Distance>;
} // namespace mtrc::stats

namespace mtrc {
using stats::properties::distance_distribution;
using stats::properties::distance_distribution_options;
} // namespace mtrc

#endif
