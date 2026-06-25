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
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/space/chunked.hpp>
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
// For large spaces, defaults may summarize a deterministic sample of pair ordinals:
// pair_count remains the full unordered pair count, while evaluated_pair_count/sample_count
// report how many distance values fed the quantiles, mean, extrema, and histogram.
//
// Non-finite distances are REJECTED with std::invalid_argument: a NaN has no defined order,
// so sorting/quantiles would be undefined. A true metric always yields finite values.
template <typename Distance> struct DistanceDistribution {
	using distance_type = Distance;

	std::size_t record_count{};
	std::size_t pair_count{};
	std::size_t evaluated_pair_count{};
	std::size_t sample_count{};
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
	core::ApproximationQuality approximation_quality;

	auto empty() const -> bool { return pair_count == 0; }
	auto bucket_count() const -> std::size_t { return histogram_counts.size(); }
};

struct distance_distribution_options {
	distance_distribution_options() = default;

	std::size_t bucket_count{10};
	std::vector<double> quantile_probabilities{0.0, 0.25, 0.5, 0.75, 1.0};
	std::size_t max_exact_pair_records{4096};
	std::size_t sample_count{4096};
	bool allow_approximate{true};
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

inline auto unordered_pair_count(std::size_t record_count) -> std::size_t
{
	if (record_count < 2) {
		return 0;
	}
	return record_count % 2 == 0 ? (record_count / 2) * (record_count - 1)
								 : record_count * ((record_count - 1) / 2);
}

inline auto sampled_pair_ordinals(std::size_t pair_count, std::size_t requested_count) -> std::vector<std::size_t>
{
	std::vector<std::size_t> ordinals;
	if (pair_count == 0 || requested_count == 0) {
		return ordinals;
	}
	const auto sample_count = std::min(pair_count, requested_count);
	ordinals.reserve(sample_count);
	for (std::size_t sample_index = 0; sample_index < sample_count; ++sample_index) {
		const auto ordinal = (sample_index * pair_count) / sample_count;
		if (ordinals.empty() || ordinals.back() != ordinal) {
			ordinals.push_back(ordinal);
		}
	}
	return ordinals;
}

inline auto pair_position_for_ordinal(std::size_t record_count, std::size_t ordinal,
									  std::size_t &row, std::size_t &row_start_ordinal)
	-> std::pair<std::size_t, std::size_t>
{
	while (row + 1 < record_count) {
		const auto row_pair_count = record_count - row - 1;
		if (ordinal < row_start_ordinal + row_pair_count) {
			return {row, row + 1 + (ordinal - row_start_ordinal)};
		}
		row_start_ordinal += row_pair_count;
		++row;
	}
	throw std::out_of_range("sampled pair ordinal is outside the pair set");
}

template <typename Provider, typename = void> struct HasDistanceAtPosition : std::false_type {};

template <typename Provider>
struct HasDistanceAtPosition<
	Provider, std::void_t<decltype(std::declval<const Provider &>().distance_at_position(
				  std::declval<std::size_t>(), std::declval<std::size_t>()))>> : std::true_type {};

template <typename Provider>
auto distance_at_pair_position(const Provider &provider, std::size_t lhs_position, std::size_t rhs_position)
	-> typename Provider::distance_type
{
	if constexpr (HasDistanceAtPosition<Provider>::value) {
		return provider.distance_at_position(lhs_position, rhs_position);
	} else {
		return provider.distance(provider.id(lhs_position), provider.id(rhs_position));
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

template <typename Distance>
auto finalize_distance_distribution(DistanceDistribution<Distance> &result,
									const distance_distribution_options &options,
									std::vector<Distance> &values, double sum) -> void
{
	if (values.empty()) {
		return;
	}

	std::sort(values.begin(), values.end());
	result.minimum = values.front();
	result.maximum = values.back();
	result.mean = sum / static_cast<double>(result.evaluated_pair_count);
	result.median = static_cast<double>(interpolated_quantile(values, 0.5));

	result.quantile_probabilities = options.quantile_probabilities;
	result.quantile_values.reserve(result.quantile_probabilities.size());
	for (const auto probability : result.quantile_probabilities) {
		result.quantile_values.push_back(interpolated_quantile(values, probability));
	}

	if (options.bucket_count > 0) {
		result.histogram_counts.assign(options.bucket_count, 0);
		result.histogram_edges.reserve(options.bucket_count + 1);
		const double minimum = static_cast<double>(result.minimum);
		const double maximum = static_cast<double>(result.maximum);
		const double span = maximum - minimum;
		for (std::size_t edge = 0; edge <= options.bucket_count; ++edge) {
			const double position =
				minimum + span * static_cast<double>(edge) / static_cast<double>(options.bucket_count);
			result.histogram_edges.push_back(static_cast<Distance>(position));
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
}

template <typename Distance>
auto mark_distribution_approximation_quality(DistanceDistribution<Distance> &result,
											 const std::vector<Distance> &values,
											 std::string candidate_policy,
											 std::string reason) -> void
{
	auto &quality = result.approximation_quality;
	quality.diagnostic = "distance_distribution_approximation";
	quality.candidate_policy = std::move(candidate_policy);
	quality.candidate_count = result.evaluated_pair_count;
	quality.candidate_universe = result.pair_count;
	quality.distance_evaluations = result.evaluated_pair_count;
	quality.sample_count = result.evaluated_pair_count;
	quality.sample_universe = result.pair_count;
	quality.candidate_fraction = result.pair_count == 0
									  ? 1.0
									  : static_cast<double>(result.evaluated_pair_count) /
											static_cast<double>(result.pair_count);
	quality.sample_fraction = quality.candidate_fraction;
	if (values.size() > 1 && result.pair_count > 1) {
		double squared_error_sum = 0.0;
		for (const auto value : values) {
			const auto difference = static_cast<double>(value) - result.mean;
			squared_error_sum += difference * difference;
		}
		const auto sample_variance = squared_error_sum / static_cast<double>(values.size() - 1);
		auto standard_error = std::sqrt(sample_variance / static_cast<double>(values.size()));
		if (values.size() < result.pair_count) {
			const auto population = static_cast<double>(result.pair_count);
			const auto sample = static_cast<double>(values.size());
			standard_error *= std::sqrt((population - sample) / (population - 1.0));
		} else {
			standard_error = 0.0;
		}
		quality.standard_error = standard_error;
		quality.confidence_radius_95 = 1.96 * standard_error;
	}
	quality.reason = std::move(reason);
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
	result.pair_count = distribution_detail::unordered_pair_count(result.record_count);
	if (result.pair_count == 0) {
		return result;
	}

	std::vector<distance_type> values;
	const auto record_count = provider.record_count();
	const auto approximate_allowed =
		options.allow_approximate && record_count > options.max_exact_pair_records;
	const auto exact_pairs =
		!approximate_allowed || result.pair_count <= options.sample_count;
	if (!exact_pairs && options.sample_count == 0) {
		throw std::invalid_argument("distance_distribution sample_count must be >= 1 when pair sampling is enabled");
	}
	values.reserve(exact_pairs ? result.pair_count : std::min(result.pair_count, options.sample_count));

	double sum = 0.0;
	auto add_distance = [&result, &sum, &values](distance_type distance) {
		if (!distribution_detail::distribution_finite_scalar(distance)) {
			throw std::invalid_argument("distance_distribution requires finite distance values");
		}
		values.push_back(distance);
		++result.evaluated_pair_count;
		sum += static_cast<double>(distance);
		if (distance <= distance_type{}) {
			++result.zero_distance_pair_count;
		} else {
			result.has_nonzero_distances = true;
		}
	};

	if (exact_pairs) {
		for (std::size_t row = 0; row < record_count; ++row) {
			for (std::size_t column = row + 1; column < record_count; ++column) {
				add_distance(distribution_detail::distance_at_pair_position(provider, row, column));
			}
		}
	} else {
		result.exact = false;
		result.algorithm = "sampled_distance_distribution";
		auto row = std::size_t{0};
		auto row_start_ordinal = std::size_t{0};
		for (const auto ordinal : distribution_detail::sampled_pair_ordinals(result.pair_count, options.sample_count)) {
			const auto pair = distribution_detail::pair_position_for_ordinal(
				record_count, ordinal, row, row_start_ordinal);
			add_distance(distribution_detail::distance_at_pair_position(provider, pair.first, pair.second));
		}
	}

	result.sample_count = result.evaluated_pair_count;
	if (values.empty()) {
		return result;
	}

	distribution_detail::finalize_distance_distribution(result, options, values, sum);

	if (!result.exact) {
		distribution_detail::mark_distribution_approximation_quality(
			result, values, "regular_pair_ordinal_sample",
			"distance distribution estimated from a deterministic regular pair sample; standard error is diagnostic");
	}

	return result;
}

template <typename Space>
auto chunked_distance_distribution(const ::mtrc::space::ChunkedSpaceView<Space> &chunks,
								   distance_distribution_options options = {})
	-> DistanceDistribution<typename ::mtrc::space::ChunkedSpaceView<Space>::distance_type>
{
	using distance_type = typename ::mtrc::space::ChunkedSpaceView<Space>::distance_type;

	DistanceDistribution<distance_type> result;
	const auto plan = chunks.plan_diagnostics();
	result.record_count = plan.record_count;
	result.pair_count = plan.dense_pair_distance_evaluations;
	result.exact = false;
	result.algorithm = "chunked_distance_distribution";
	result.representation = "chunked_space_view";

	std::vector<distance_type> values;
	values.reserve(plan.bounded_pair_distance_evaluations);
	double sum = 0.0;
	auto add_distance = [&result, &sum, &values](distance_type distance) {
		if (!distribution_detail::distribution_finite_scalar(distance)) {
			throw std::invalid_argument("chunked_distance_distribution requires finite distance values");
		}
		values.push_back(distance);
		++result.evaluated_pair_count;
		sum += static_cast<double>(distance);
		if (distance <= distance_type{}) {
			++result.zero_distance_pair_count;
		} else {
			result.has_nonzero_distances = true;
		}
	};

	chunks.for_each_local_pair([&](std::size_t, auto, auto, distance_type distance) {
		add_distance(distance);
	});
	chunks.for_each_representative_pair([&](std::size_t, std::size_t, auto, auto, distance_type distance) {
		add_distance(distance);
	});

	result.sample_count = result.evaluated_pair_count;
	distribution_detail::finalize_distance_distribution(result, options, values, sum);
	distribution_detail::mark_distribution_approximation_quality(
		result, values, "local_chunks_plus_representative_pairs",
		"distance distribution estimated from local chunk pairs plus representative pairs; standard error is diagnostic");

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

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto distance_distribution(const Container &records, const Metric &metric, distance_distribution_options options = {})
	-> DistanceDistribution<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = distance_distribution(space, std::move(options));
	result.representation = "records";
	return result;
}

} // namespace mtrc::stats::properties

namespace mtrc::stats {
using properties::chunked_distance_distribution;
using properties::distance_distribution;
using properties::distance_distribution_options;
template <typename Distance> using DistanceDistribution = properties::DistanceDistribution<Distance>;
} // namespace mtrc::stats

namespace mtrc {
using stats::properties::chunked_distance_distribution;
using stats::properties::distance_distribution;
using stats::properties::distance_distribution_options;
} // namespace mtrc

#endif
