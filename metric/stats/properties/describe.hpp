// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_PROPERTIES_DESCRIBE_HPP
#define _METRIC_STATS_PROPERTIES_DESCRIBE_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/space/storage/distance_matrix.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>

namespace mtrc::stats::properties {

struct describe_options {
	std::size_t max_exact_pair_summary_records{4096};
	std::size_t pair_summary_sample_count{4096};
	std::size_t max_exact_intrinsic_records{256};
	std::size_t intrinsic_sample_count{64};
	std::size_t intrinsic_radius_sample_count{64};
	bool allow_approximate_pair_summary{true};
	bool allow_approximate_intrinsic_dimension{true};
};

namespace describe_detail {

inline auto sampled_positions(std::size_t count, std::size_t requested_count) -> std::vector<std::size_t>
{
	std::vector<std::size_t> positions;
	if (count == 0 || requested_count == 0) {
		return positions;
	}
	const auto sample_count = std::min(count, requested_count);
	positions.reserve(sample_count);
	for (std::size_t sample_index = 0; sample_index < sample_count; ++sample_index) {
		const auto position = (sample_index * count) / sample_count;
		if (positions.empty() || positions.back() != position) {
			positions.push_back(position);
		}
	}
	return positions;
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

template <typename Distance> struct pair_summary {
	std::size_t total_pair_count{};
	std::size_t evaluated_pair_count{};
	std::size_t zero_distance_pair_count{};
	Distance minimum_nonzero_distance{};
	Distance maximum_distance{};
	double average_distance{};
	bool has_nonzero_distances{};
	bool exact{true};
};

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto summarize_pairs(const Provider &provider, const describe_options &options)
	-> pair_summary<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	pair_summary<distance_type> summary;
	summary.total_pair_count = unordered_pair_count(provider.record_count());
	if (summary.total_pair_count == 0) {
		return summary;
	}

	const auto exact_pairs =
		provider.record_count() <= options.max_exact_pair_summary_records ||
		!options.allow_approximate_pair_summary;
	if (!exact_pairs && options.pair_summary_sample_count == 0) {
		throw std::invalid_argument("describe_structure pair_summary_sample_count must be >= 1 when pair sampling is enabled");
	}

	core::ScalarAccumulator<distance_type> distance_summary;
	auto add_distance = [&summary, &distance_summary](distance_type distance) {
		++summary.evaluated_pair_count;
		distance_summary.add(distance);
		if (distance <= distance_type{}) {
			++summary.zero_distance_pair_count;
		} else if (!summary.has_nonzero_distances || distance < summary.minimum_nonzero_distance) {
			summary.minimum_nonzero_distance = distance;
			summary.has_nonzero_distances = true;
		}
	};

	if (exact_pairs) {
		for (std::size_t row = 0; row < provider.record_count(); ++row) {
			for (std::size_t column = row + 1; column < provider.record_count(); ++column) {
				add_distance(distance_at_pair_position(provider, row, column));
			}
		}
	} else {
		summary.exact = false;
		auto row = std::size_t{0};
		auto row_start_ordinal = std::size_t{0};
		for (const auto ordinal : sampled_pair_ordinals(summary.total_pair_count, options.pair_summary_sample_count)) {
			const auto pair = pair_position_for_ordinal(
				provider.record_count(), ordinal, row, row_start_ordinal);
			add_distance(distance_at_pair_position(provider, pair.first, pair.second));
		}
	}

	summary.maximum_distance = distance_summary.maximum_or();
	summary.average_distance = distance_summary.average_or();
	return summary;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto sampled_expansion_dimension(const Provider &provider, const describe_options &options)
	-> double
{
	using distance_type = typename Provider::distance_type;
	if (provider.record_count() < 2) {
		return 0.0;
	}

	const auto center_positions = sampled_positions(provider.record_count(), options.intrinsic_sample_count);
	double maximum_dimension = 0.0;
	for (const auto center_position : center_positions) {
		std::vector<distance_type> distances;
		distances.reserve(provider.record_count());
		for (std::size_t candidate = 0; candidate < provider.record_count(); ++candidate) {
			const auto distance = distance_at_pair_position(provider, center_position, candidate);
			if (distance > distance_type{}) {
				distances.push_back(distance);
			}
		}
		if (distances.empty()) {
			continue;
		}
		std::sort(distances.begin(), distances.end());
		const auto radius_positions = sampled_positions(distances.size(), options.intrinsic_radius_sample_count);
		for (const auto radius_position : radius_positions) {
			const auto radius = distances[radius_position];
			const auto outer_radius = radius + radius;
			const auto inner_count = static_cast<std::size_t>(
				std::upper_bound(distances.begin(), distances.end(), radius) - distances.begin()) + 1;
			const auto outer_count = static_cast<std::size_t>(
				std::upper_bound(distances.begin(), distances.end(), outer_radius) - distances.begin()) + 1;
			if (inner_count > 0 && outer_count >= inner_count) {
				maximum_dimension = std::max(
					maximum_dimension,
					std::log(static_cast<double>(outer_count) / static_cast<double>(inner_count)) / std::log(2.0));
			}
		}
	}
	return maximum_dimension;
}

} // namespace describe_detail

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto describe_structure(const Provider &provider, describe_options options = {})
	-> StructureDescription<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	const auto pairs = describe_detail::summarize_pairs(provider, options);

	const auto exact_intrinsic =
		provider.record_count() <= options.max_exact_intrinsic_records ||
		!options.allow_approximate_intrinsic_dimension;
	const auto intrinsic_dimension = exact_intrinsic
		? core::expansion_dimension(space::storage::provider_symmetric_distance_matrix(provider),
									provider.record_count(), distance_type{})
		: describe_detail::sampled_expansion_dimension(provider, options);
	auto result = core::make_structure_description(
		provider.record_count(), pairs.total_pair_count, pairs.zero_distance_pair_count,
		pairs.minimum_nonzero_distance, pairs.maximum_distance, pairs.average_distance, intrinsic_dimension,
		pairs.has_nonzero_distances, "pairwise_distances", pairs.exact && exact_intrinsic);
	if (pairs.exact && !exact_intrinsic) {
		result.strategy = "exact_pairs_sampled_intrinsic_dimension";
	} else if (!pairs.exact && exact_intrinsic) {
		result.strategy = "sampled_pairs_exact_intrinsic_dimension";
	} else if (!pairs.exact && !exact_intrinsic) {
		result.strategy = "sampled_pairs_sampled_intrinsic_dimension";
	}
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto describe_structure(const Space &space, describe_options options = {}) -> StructureDescription<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	auto result = describe_structure(provider, std::move(options));
	result.representation = "metric_space";
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto describe_structure(const Container &records, const Metric &metric)
	-> StructureDescription<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = describe_structure(space);
	result.representation = "records";
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto describe_structure(const Container &records, const Metric &metric, describe_options options)
	-> StructureDescription<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = describe_structure(space, std::move(options));
	result.representation = "records";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto describe_structure(const Space &space, space::storage::policy runtime_policy)
	-> StructureDescription<typename Space::distance_type>
{
	space::storage::require_exact_describe(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		auto matrix = space::storage::make_distance_table(space, runtime_policy);
		auto result = describe_structure(matrix);
		result.representation = space::storage::describe_representation(runtime_policy);
		return result;
	}

	auto result = describe_structure(space);
	result.representation = space::storage::describe_representation(runtime_policy);
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto describe_structure(const Container &records, const Metric &metric, space::storage::policy runtime_policy)
	-> StructureDescription<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	return describe_structure(space, runtime_policy);
}

} // namespace mtrc::stats::properties

namespace mtrc {
using stats::properties::describe_options;
using stats::properties::describe_structure;
} // namespace mtrc

#endif
