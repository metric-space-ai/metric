// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_CORRELATE_COMPARE_HPP
#define _METRIC_STATS_CORRELATE_COMPARE_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/result.hpp>
#include <metric/stats/correlate/correlation.hpp>
#include <metric/space/chunked.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/stats/correlate/options.hpp>

namespace mtrc {

namespace compare_detail {

constexpr std::size_t max_default_exact_mgc_records = 4096;

inline auto default_mgc_compare_preflight_policy() -> space::storage::policy
{
	return space::storage::allow_approximate_fallback(space::storage::with_distance_table_budget(
		space::storage::materialized(space::storage::exact()), max_default_exact_mgc_records, 0));
}

inline auto plan_failure_message(const space::storage::execution_plan &plan, const char *fallback) -> std::string
{
	auto message = plan.reason.empty() ? std::string("runtime policy refused by resource budget") : plan.reason;
	if (!plan.fallback_hint.empty()) {
		message += "; fallback: " + plan.fallback_hint;
	} else if (fallback != nullptr && fallback[0] != '\0') {
		message += "; fallback: ";
		message += fallback;
	}
	return message;
}

template <typename Space>
auto estimate_materialized_compare_plan(const Space &space, space::storage::policy runtime_policy)
	-> space::storage::execution_plan
{
	auto plan = space::storage::estimate_cost(space, "compare", runtime_policy);
	if (!plan.refused && runtime_policy.uses_materialization() &&
		!space::storage::uses_distance_table_materialization(runtime_policy)) {
		plan = space::storage::estimate_cost(space, "compare", space::storage::using_distance_table(runtime_policy));
	}
	return plan;
}

inline auto uses_sampled_compare_fallback(const space::storage::execution_plan &plan) -> bool
{
	return plan.allowed && plan.downgraded && !plan.exact && plan.representation == "metric_space_sample";
}

inline auto uses_chunked_compare_fallback(const space::storage::execution_plan &plan) -> bool
{
	return plan.allowed && plan.downgraded && !plan.exact && plan.representation == "chunked_space_view";
}

inline auto ensure_supported_materialized_plan(const space::storage::execution_plan &plan) -> void
{
	if (plan.refused) {
		throw RepresentationError(plan_failure_message(
			plan, "use space::storage::using_implicit(), an approximate policy, or raise the resource_budget"));
	}
	if (plan.downgraded && !space::storage::uses_blocked_exact_fallback(plan) &&
		!uses_sampled_compare_fallback(plan) && !uses_chunked_compare_fallback(plan)) {
		auto message = plan_failure_message(plan, "use a sampled MGC estimate or raise the resource_budget");
		message += "; executor integration for this downgraded route is not implemented";
		throw RepresentationError(message);
	}
}

inline auto bounded_mgc_estimate_options(std::size_t record_count, stats::correlate::mgc_options strategy)
	-> stats::correlate::mgc_options
{
	if (record_count > max_default_exact_mgc_records && strategy.sample_count >= record_count) {
		strategy.sample_count = std::min(stats::correlate::mgc_options{}.sample_count, record_count - 1);
	}
	return strategy;
}

inline auto compare_fallback_reason(const space::storage::execution_plan &left_plan,
									const space::storage::execution_plan &right_plan) -> std::string
{
	if (!left_plan.reason.empty()) {
		return left_plan.reason;
	}
	if (!right_plan.reason.empty()) {
		return right_plan.reason;
	}
	return "default compare guard selected subsampled MGC before dense all-pairs execution";
}

inline auto add_unique_sample_position(std::vector<std::size_t> &positions, std::size_t position,
									   std::size_t record_count) -> void
{
	if (record_count == 0) {
		return;
	}
	position = std::min(position, record_count - 1);
	if (std::find(positions.begin(), positions.end(), position) == positions.end()) {
		positions.push_back(position);
	}
}

inline auto chunked_compare_sample_positions(std::size_t record_count, std::size_t chunk_size,
											 std::size_t sample_count) -> std::vector<std::size_t>
{
	std::vector<std::size_t> positions;
	if (record_count == 0 || sample_count == 0) {
		return positions;
	}

	chunk_size = chunk_size == 0 ? std::size_t{1} : chunk_size;
	sample_count = std::min(sample_count, record_count);
	const auto chunk_count = (record_count + chunk_size - 1) / chunk_size;
	const auto representative_samples = std::min(sample_count, chunk_count);
	positions.reserve(sample_count);
	for (std::size_t sample = 0; sample < representative_samples; ++sample) {
		const auto chunk_index = (sample * chunk_count) / representative_samples;
		add_unique_sample_position(positions, chunk_index * chunk_size, record_count);
	}
	for (std::size_t sample = 0; positions.size() < sample_count && sample < sample_count; ++sample) {
		add_unique_sample_position(positions, (sample * record_count) / sample_count, record_count);
	}
	for (std::size_t position = 0; positions.size() < sample_count && position < record_count; ++position) {
		add_unique_sample_position(positions, position, record_count);
	}
	std::sort(positions.begin(), positions.end());
	return positions;
}

inline auto chunked_compare_sample_count_from_plan(const space::storage::execution_plan &plan,
												   stats::correlate::mgc_options strategy) -> std::size_t
{
	if (strategy.sample_count == 0) {
		throw std::invalid_argument("chunked MGC estimate sample_count must be >= 1");
	}
	mtrc::space::chunked_work_diagnostics diagnostics;
	diagnostics.record_count = plan.record_count;
	diagnostics.representative_count = plan.representative_count == 0 ? plan.chunk_count : plan.representative_count;
	return space::storage::chunked_compare_sample_count(diagnostics, plan.budget, strategy.sample_count);
}

inline auto chunked_compare_chunk_size(const space::storage::execution_plan &left_plan,
									   const space::storage::execution_plan &right_plan) -> std::size_t
{
	if (left_plan.chunk_size == 0) {
		return right_plan.chunk_size == 0 ? std::size_t{1} : right_plan.chunk_size;
	}
	if (right_plan.chunk_size == 0) {
		return left_plan.chunk_size;
	}
	return std::min(left_plan.chunk_size, right_plan.chunk_size);
}

template <typename LeftSpace, typename RightSpace>
auto chunked_mgc_estimate(const LeftSpace &left_space, const RightSpace &right_space,
						  stats::correlate::mgc_options strategy,
						  const space::storage::execution_plan &left_plan,
						  const space::storage::execution_plan &right_plan) -> CorrelationResult<double>
{
	if (left_space.size() != right_space.size()) {
		throw std::invalid_argument("chunked MGC estimate requires spaces with the same record count");
	}
	if (left_space.size() < 2) {
		throw std::invalid_argument("chunked MGC estimate requires at least two paired records");
	}

	const auto left_sample_count = uses_chunked_compare_fallback(left_plan)
									   ? chunked_compare_sample_count_from_plan(left_plan, strategy)
									   : left_space.size();
	const auto right_sample_count = uses_chunked_compare_fallback(right_plan)
										? chunked_compare_sample_count_from_plan(right_plan, strategy)
										: right_space.size();
	const auto sample_count = std::min(left_sample_count, right_sample_count);
	const auto positions = chunked_compare_sample_positions(
		left_space.size(), chunked_compare_chunk_size(left_plan, right_plan), sample_count);
	if (positions.size() < 2) {
		throw std::invalid_argument("chunked MGC estimate requires at least two sampled records");
	}

	std::vector<typename LeftSpace::record_type> left_records;
	std::vector<typename RightSpace::record_type> right_records;
	left_records.reserve(positions.size());
	right_records.reserve(positions.size());
	const auto &left_all_records = left_space.records();
	const auto &right_all_records = right_space.records();
	for (const auto position : positions) {
		left_records.push_back(left_all_records[position]);
		right_records.push_back(right_all_records[position]);
	}

	auto result = stats::correlate::mgc(left_records, left_space.metric(), right_records, right_space.metric());
	result.left_record_count = left_space.size();
	result.right_record_count = right_space.size();
	result.exact = false;
	result.algorithm = "chunked_mgc_estimate";
	result.left_representation = "chunked_space_view";
	result.right_representation = "chunked_space_view";
	result.sample_count = positions.size();
	result.sample_iterations = 1;
	const auto chunk_count = std::max(left_plan.chunk_count, right_plan.chunk_count);
	const auto bounded_pair_distance_evaluations =
		std::max(left_plan.bounded_pair_distance_evaluations, right_plan.bounded_pair_distance_evaluations);
	const auto dense_pair_distance_evaluations =
		std::max(left_plan.dense_pair_distance_evaluations, right_plan.dense_pair_distance_evaluations);
	result.approximation_reason =
		"chunked compare fallback selected a bounded paired representative sample before dense all-pairs MGC; "
		"chunk_size=" + std::to_string(chunked_compare_chunk_size(left_plan, right_plan)) +
		" chunk_count=" + std::to_string(chunk_count) +
		" sample_count=" + std::to_string(positions.size()) +
		" bounded_pair_distance_evaluations=" + std::to_string(bounded_pair_distance_evaluations) +
		" dense_pair_distance_evaluations=" + std::to_string(dense_pair_distance_evaluations);
	return result;
}

template <typename LeftSpace, typename RightSpace>
auto should_use_default_mgc_estimate(const LeftSpace &left_space, const RightSpace &right_space) -> bool
{
	const auto runtime_policy = default_mgc_compare_preflight_policy();
	const auto left_plan = space::storage::estimate_cost(left_space, "compare", runtime_policy);
	const auto right_plan = space::storage::estimate_cost(right_space, "compare", runtime_policy);
	if (left_plan.refused) {
		throw RepresentationError(plan_failure_message(left_plan, "use an explicit approximate policy or reduce record count"));
	}
	if (right_plan.refused) {
		throw RepresentationError(plan_failure_message(right_plan, "use an explicit approximate policy or reduce record count"));
	}
	return uses_sampled_compare_fallback(left_plan) || uses_sampled_compare_fallback(right_plan);
}

template <typename LeftSpace, typename RightSpace>
auto compare_with_materialized_providers(const LeftSpace &left_space, const RightSpace &right_space,
										 stats::correlate::mgc_options strategy,
										 space::storage::policy runtime_policy) -> CorrelationResult<double>
{
	const auto left_plan = estimate_materialized_compare_plan(left_space, runtime_policy);
	const auto right_plan = estimate_materialized_compare_plan(right_space, runtime_policy);
	ensure_supported_materialized_plan(left_plan);
	ensure_supported_materialized_plan(right_plan);

	if (uses_chunked_compare_fallback(left_plan) || uses_chunked_compare_fallback(right_plan)) {
		return chunked_mgc_estimate(left_space, right_space, strategy, left_plan, right_plan);
	}

	if (uses_sampled_compare_fallback(left_plan) || uses_sampled_compare_fallback(right_plan)) {
		auto result = stats::correlate::mgc_estimate(
			left_space, right_space, bounded_mgc_estimate_options(left_space.size(), strategy));
		if (!result.exact) {
			result.approximation_reason = compare_fallback_reason(left_plan, right_plan);
		}
		return result;
	}

	const auto use_blocked_provider =
		space::storage::uses_blocked_exact_fallback(left_plan) ||
		space::storage::uses_blocked_exact_fallback(right_plan);

	if (use_blocked_provider) {
		auto left_matrix = space::storage::make_blocked_distance_table(left_space, runtime_policy);
		auto right_matrix = space::storage::make_blocked_distance_table(right_space, runtime_policy);
		auto result = stats::correlate::mgc(left_matrix, right_matrix);
		result.left_representation = "blocked_distance_table";
		result.right_representation = "blocked_distance_table";
		return result;
	}

	auto left_matrix = space::storage::make_distance_table(left_space, runtime_policy);
	auto right_matrix = space::storage::make_distance_table(right_space, runtime_policy);
	auto result = stats::correlate::mgc(left_matrix, right_matrix);
	result.left_representation = space::storage::compare_representation(runtime_policy);
	result.right_representation = space::storage::compare_representation(runtime_policy);
	return result;
}

} // namespace compare_detail

// compare()/correlate() test DEPENDENCE between two paired finite metric spaces. They
// are not metrics and return a CorrelationResult carrying a single dependence statistic
// (the MGC sample statistic in [-1, 1]); no p-value is produced. The two spaces must be
// aligned by observation and share the same record count. See
// stats/correlate/correlation.hpp for the full statistical contract.
template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto compare(const LeftSpace &left_space, const RightSpace &right_space, stats::correlate::mgc_options strategy) -> CorrelationResult<double>
{
	if (compare_detail::should_use_default_mgc_estimate(left_space, right_space)) {
		const auto left_plan =
			space::storage::estimate_cost(left_space, "compare", compare_detail::default_mgc_compare_preflight_policy());
		const auto right_plan =
			space::storage::estimate_cost(right_space, "compare", compare_detail::default_mgc_compare_preflight_policy());
		auto result = stats::correlate::mgc_estimate(
			left_space, right_space,
			compare_detail::bounded_mgc_estimate_options(left_space.size(), strategy));
		if (!result.exact) {
			result.approximation_reason = compare_detail::compare_fallback_reason(left_plan, right_plan);
		}
		return result;
	}
	return stats::correlate::mgc(left_space, right_space);
}

template <typename LeftProvider, typename RightProvider,
		  typename std::enable_if<PairwiseDistances_v<LeftProvider> && PairwiseDistances_v<RightProvider>, int>::type = 0>
auto compare(const LeftProvider &left_provider, const RightProvider &right_provider, stats::correlate::mgc_options)
	-> CorrelationResult<double>
{
	return stats::correlate::mgc(left_provider, right_provider);
}

template <typename LeftContainer, typename LeftMetric, typename RightContainer, typename RightMetric>
auto compare(const LeftContainer &left_records, const LeftMetric &left_metric, const RightContainer &right_records,
			 const RightMetric &right_metric, stats::correlate::mgc_options) -> CorrelationResult<double>
{
	return stats::correlate::mgc(left_records, left_metric, right_records, right_metric);
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto compare(const LeftSpace &left_space, const RightSpace &right_space, stats::correlate::mgc_options strategy,
			 space::storage::policy runtime_policy) -> CorrelationResult<double>
{
	space::storage::require_parallel_metric<typename LeftSpace::metric_type>(runtime_policy);
	space::storage::require_parallel_metric<typename RightSpace::metric_type>(runtime_policy);

	if (!runtime_policy.is_exact()) {
		return stats::correlate::mgc_estimate(left_space, right_space, strategy);
	}

	if (runtime_policy.uses_materialization()) {
		return compare_detail::compare_with_materialized_providers(left_space, right_space, strategy, runtime_policy);
	}

	auto result = compare(left_space, right_space, strategy);
	if (result.exact) {
		result.left_representation = space::storage::compare_representation(runtime_policy);
		result.right_representation = space::storage::compare_representation(runtime_policy);
	}
	return result;
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto compare(const LeftSpace &left_space, const RightSpace &right_space) -> CorrelationResult<double>
{
	return compare(left_space, right_space, stats::correlate::mgc_options{});
}

template <typename LeftProvider, typename RightProvider,
		  typename std::enable_if<PairwiseDistances_v<LeftProvider> && PairwiseDistances_v<RightProvider>, int>::type = 0>
auto compare(const LeftProvider &left_provider, const RightProvider &right_provider) -> CorrelationResult<double>
{
	return compare(left_provider, right_provider, stats::correlate::mgc_options{});
}

template <typename LeftContainer, typename LeftMetric, typename RightContainer, typename RightMetric>
auto compare(const LeftContainer &left_records, const LeftMetric &left_metric, const RightContainer &right_records,
			 const RightMetric &right_metric) -> CorrelationResult<double>
{
	return compare(left_records, left_metric, right_records, right_metric, stats::correlate::mgc_options{});
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto compare(const LeftSpace &left_space, const RightSpace &right_space, space::storage::policy runtime_policy)
	-> CorrelationResult<double>
{
	return compare(left_space, right_space, stats::correlate::mgc_options{}, runtime_policy);
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto correlate(const LeftSpace &left_space, const RightSpace &right_space, stats::correlate::mgc_options strategy)
	-> CorrelationResult<double>
{
	return compare(left_space, right_space, strategy);
}

template <typename LeftProvider, typename RightProvider,
		  typename std::enable_if<PairwiseDistances_v<LeftProvider> && PairwiseDistances_v<RightProvider>, int>::type = 0>
auto correlate(const LeftProvider &left_provider, const RightProvider &right_provider,
			   stats::correlate::mgc_options strategy) -> CorrelationResult<double>
{
	return compare(left_provider, right_provider, strategy);
}

template <typename LeftContainer, typename LeftMetric, typename RightContainer, typename RightMetric>
auto correlate(const LeftContainer &left_records, const LeftMetric &left_metric, const RightContainer &right_records,
			   const RightMetric &right_metric, stats::correlate::mgc_options strategy) -> CorrelationResult<double>
{
	return compare(left_records, left_metric, right_records, right_metric, strategy);
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto correlate(const LeftSpace &left_space, const RightSpace &right_space, stats::correlate::mgc_options strategy,
			   space::storage::policy runtime_policy) -> CorrelationResult<double>
{
	return compare(left_space, right_space, strategy, runtime_policy);
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto correlate(const LeftSpace &left_space, const RightSpace &right_space) -> CorrelationResult<double>
{
	return correlate(left_space, right_space, stats::correlate::mgc_options{});
}

template <typename LeftProvider, typename RightProvider,
		  typename std::enable_if<PairwiseDistances_v<LeftProvider> && PairwiseDistances_v<RightProvider>, int>::type = 0>
auto correlate(const LeftProvider &left_provider, const RightProvider &right_provider) -> CorrelationResult<double>
{
	return correlate(left_provider, right_provider, stats::correlate::mgc_options{});
}

template <typename LeftContainer, typename LeftMetric, typename RightContainer, typename RightMetric>
auto correlate(const LeftContainer &left_records, const LeftMetric &left_metric, const RightContainer &right_records,
			   const RightMetric &right_metric) -> CorrelationResult<double>
{
	return correlate(left_records, left_metric, right_records, right_metric, stats::correlate::mgc_options{});
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto correlate(const LeftSpace &left_space, const RightSpace &right_space, space::storage::policy runtime_policy)
	-> CorrelationResult<double>
{
	return correlate(left_space, right_space, stats::correlate::mgc_options{}, runtime_policy);
}

} // namespace mtrc

#endif
