// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_CORRELATE_SIGNIFICANCE_HPP
#define _METRIC_STATS_CORRELATE_SIGNIFICANCE_HPP

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/correlation/mgc.hpp>
#include <metric/record/id.hpp>
#include <metric/space/storage/distance_matrix.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/stats/correlate/correlation.hpp>

namespace mtrc::stats::correlate {

// Significance and alignment for the MGC dependence statistic. MGC is a DEPENDENCE
// statistic between two paired finite metric spaces, NEVER a metric or distance. These
// helpers add (1) a permutation/significance workflow that turns the point statistic into
// an upper-tail p-value, and (2) RecordId alignment so two spaces can be paired by shared
// record identity (with explicit dropped-pair reporting) instead of only by position.

// Result of a permutation significance test for MGC. The permutation test is a Monte-Carlo
// estimate over `permutations` random relabelings of the right space (not all n!), so
// `exact` is false. p_value is the upper-tail estimate (1 + #{perm stat >= observed}) /
// (permutations + 1): small p_value => the observed dependence is unlikely under
// independence. `statistic` is the same MGC sample statistic in [-1, 1] as mgc().
template <typename Value = double> struct SignificanceResult {
	using value_type = Value;

	Value statistic{};
	double p_value{1.0};
	std::size_t permutations{};
	std::size_t ge_count{};
	std::uint64_t seed{};
	std::size_t record_count{};
	bool exact{false};
	std::string algorithm{"mgc_permutation"};
	std::string left_representation;
	std::string right_representation;
};

struct significance_options {
	significance_options() = default;
	explicit significance_options(std::size_t permutations, std::uint64_t seed = 0x9E3779B97F4A7C15ULL)
		: permutations(permutations), seed(seed)
	{
	}

	std::size_t permutations{200};
	std::uint64_t seed{0x9E3779B97F4A7C15ULL};
	// Maximum dense matrix cells copied or processed by permutation trials.
	// Set to 0 only when unbounded permutation work is intentional.
	std::size_t max_permutation_matrix_cells{100'000'000};
};

namespace significance_detail {

constexpr std::size_t max_default_exact_mgc_significance_records = 2886;

inline auto default_mgc_significance_preflight_policy() -> space::storage::policy
{
	return space::storage::with_distance_table_budget(
		space::storage::materialized(space::storage::exact()), max_default_exact_mgc_significance_records, 0);
}

inline auto significance_plan_failure_message(const space::storage::execution_plan &plan) -> std::string
{
	auto message = plan.reason.empty() ? std::string("MGC significance refused by resource budget") : plan.reason;
	if (!plan.fallback_hint.empty()) {
		message += "; fallback: " + plan.fallback_hint;
	}
	message += "; MGC significance requires dense exact pairwise distances, so reduce record count or pass an explicit "
			   "pairwise-distance provider";
	return message;
}

inline auto checked_significance_work_product(std::size_t lhs, std::size_t rhs, const char *message)
	-> std::size_t
{
	if (rhs != 0 && lhs > std::numeric_limits<std::size_t>::max() / rhs) {
		throw mtrc::RepresentationError(message);
	}
	return lhs * rhs;
}

inline auto estimate_permutation_matrix_cells(std::size_t record_count, std::size_t permutations) -> std::size_t
{
	const auto matrix_cells = checked_significance_work_product(
		record_count, record_count, "MGC significance permutation matrix-cell estimate exceeds size_t capacity");
	return checked_significance_work_product(
		matrix_cells, permutations, "MGC significance permutation matrix-cell estimate exceeds size_t capacity");
}

inline auto require_permutation_work_budget(std::size_t record_count, significance_options options) -> void
{
	const auto estimated_cells = estimate_permutation_matrix_cells(record_count, options.permutations);
	if (options.max_permutation_matrix_cells == 0 || estimated_cells <= options.max_permutation_matrix_cells) {
		return;
	}
	throw mtrc::RepresentationError(
		"MGC significance refused permutation work before dense matrix construction: records=" +
		std::to_string(record_count) + ", permutations=" + std::to_string(options.permutations) +
		", estimated_permutation_matrix_cells=" + std::to_string(estimated_cells) +
		", max_permutation_matrix_cells=" + std::to_string(options.max_permutation_matrix_cells) +
		". Reduce permutations or set max_permutation_matrix_cells=0 only when unbounded permutation work is "
		"intentional.");
}

template <typename Space> inline auto require_default_mgc_significance_preflight(const Space &space) -> void
{
	const auto plan = space::storage::estimate_cost(
		space, "compare", default_mgc_significance_preflight_policy());
	if (plan.refused) {
		throw mtrc::RepresentationError(significance_plan_failure_message(plan));
	}
}

inline auto symmetric_double_matrix_is_finite(const mtrc::DistanceMatrix<double> &matrix) -> bool
{
	const std::size_t count = matrix.rows();
	for (std::size_t row = 0; row < count; ++row) {
		for (std::size_t column = row; column < count; ++column) {
			if (!std::isfinite(matrix(row, column))) {
				return false;
			}
		}
	}
	return true;
}

// Builds the distance matrix of the right space relabeled by `permutation`:
// Rp(i, j) = R(permutation[i], permutation[j]). R is symmetric, so the result is too.
inline auto permuted_symmetric_matrix(const mtrc::DistanceMatrix<double> &matrix,
									  const std::vector<std::size_t> &permutation) -> mtrc::DistanceMatrix<double>
{
	const std::size_t count = matrix.rows();
	mtrc::DistanceMatrix<double> permuted(count);
	for (std::size_t row = 0; row < count; ++row) {
		permuted(row, row) = 0.0;
		for (std::size_t column = row + 1; column < count; ++column) {
			permuted(row, column) = matrix(permutation[row], permutation[column]);
		}
	}
	return permuted;
}

// Deterministic in-place Fisher-Yates shuffle using a raw 64-bit Mersenne Twister stream.
// std::shuffle / std::uniform_int_distribution are intentionally avoided because their
// output is not portable across standard-library implementations; a raw modulo draw keeps
// the permutation sequence reproducible for a given seed.
inline auto fisher_yates(std::vector<std::size_t> &order, std::mt19937_64 &generator) -> void
{
	for (std::size_t index = order.size(); index > 1; --index) {
		const std::size_t swap_with = static_cast<std::size_t>(generator() % static_cast<std::uint64_t>(index));
		std::swap(order[index - 1], order[swap_with]);
	}
}

inline auto run_mgc_permutation(const mtrc::DistanceMatrix<double> &left, const mtrc::DistanceMatrix<double> &right,
								significance_options options, std::string left_representation,
								std::string right_representation) -> SignificanceResult<double>
{
	if (!symmetric_double_matrix_is_finite(left) || !symmetric_double_matrix_is_finite(right)) {
		throw std::invalid_argument("MGC significance requires finite distance values");
	}
	require_permutation_work_budget(left.rows(), options);

	SignificanceResult<double> result;
	result.permutations = options.permutations;
	result.seed = options.seed;
	result.record_count = left.rows();
	result.left_representation = std::move(left_representation);
	result.right_representation = std::move(right_representation);

	MGC_direct direct;
	result.statistic = direct(left, right);

	if (options.permutations == 0) {
		result.p_value = 1.0;
		return result;
	}

	std::mt19937_64 generator(options.seed);
	std::vector<std::size_t> order(left.rows());
	for (std::size_t index = 0; index < order.size(); ++index) {
		order[index] = index;
	}

	std::size_t ge_count = 0;
	for (std::size_t trial = 0; trial < options.permutations; ++trial) {
		fisher_yates(order, generator);
		const auto permuted_right = permuted_symmetric_matrix(right, order);
		const auto permuted_statistic = direct(left, permuted_right);
		if (permuted_statistic >= result.statistic) {
			++ge_count;
		}
	}

	result.ge_count = ge_count;
	result.p_value = static_cast<double>(ge_count + 1) / static_cast<double>(options.permutations + 1);
	return result;
}

} // namespace significance_detail

template <typename LeftProvider, typename RightProvider,
		  typename std::enable_if<PairwiseDistances_v<LeftProvider> && PairwiseDistances_v<RightProvider>, int>::type = 0>
auto mgc_significance(const LeftProvider &left_provider, const RightProvider &right_provider,
					  significance_options options = {}) -> SignificanceResult<double>
{
	if (left_provider.record_count() != right_provider.record_count()) {
		throw std::invalid_argument("MGC significance requires spaces with the same record count");
	}
	if (left_provider.record_count() < 2) {
		throw std::invalid_argument("MGC significance requires at least two paired records");
	}
	significance_detail::require_permutation_work_budget(left_provider.record_count(), options);

	auto left = space::storage::provider_symmetric_distance_matrix<double>(left_provider);
	auto right = space::storage::provider_symmetric_distance_matrix<double>(right_provider);
	return significance_detail::run_mgc_permutation(left, right, options, "pairwise_distances", "pairwise_distances");
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto mgc_significance(const LeftSpace &left_space, const RightSpace &right_space, significance_options options = {})
	-> SignificanceResult<double>
{
	if (left_space.size() != right_space.size()) {
		throw std::invalid_argument("MGC significance requires spaces with the same record count");
	}
	if (left_space.size() < 2) {
		throw std::invalid_argument("MGC significance requires at least two paired records");
	}
	significance_detail::require_permutation_work_budget(left_space.size(), options);

	significance_detail::require_default_mgc_significance_preflight(left_space);
	significance_detail::require_default_mgc_significance_preflight(right_space);

	space::storage::LiveDistances<LeftSpace> left_provider(left_space);
	space::storage::LiveDistances<RightSpace> right_provider(right_space);
	auto left_matrix = space::storage::provider_symmetric_distance_matrix<double>(left_provider);
	auto right_matrix = space::storage::provider_symmetric_distance_matrix<double>(right_provider);
	return significance_detail::run_mgc_permutation(left_matrix, right_matrix, options, "metric_space", "metric_space");
}

template <typename LeftContainer, typename LeftMetric, typename RightContainer, typename RightMetric,
		  typename LeftRecord = typename std::decay<typename LeftContainer::value_type>::type,
		  typename RightRecord = typename std::decay<typename RightContainer::value_type>::type,
		  typename std::enable_if<
			  MetricCallable_v<LeftMetric, LeftRecord> && MetricCallable_v<RightMetric, RightRecord>, int>::type = 0>
auto mgc_significance(const LeftContainer &left_records, const LeftMetric &left_metric,
					  const RightContainer &right_records, const RightMetric &right_metric,
					  significance_options options = {}) -> SignificanceResult<double>
{
	using left_metric_type = typename std::decay<LeftMetric>::type;
	using right_metric_type = typename std::decay<RightMetric>::type;
	core::MetricSpace<LeftRecord, left_metric_type> left_space(
		std::vector<LeftRecord>(left_records.begin(), left_records.end()), left_metric_type(left_metric));
	core::MetricSpace<RightRecord, right_metric_type> right_space(
		std::vector<RightRecord>(right_records.begin(), right_records.end()), right_metric_type(right_metric));
	auto result = mgc_significance(left_space, right_space, options);
	result.left_representation = "records";
	result.right_representation = "records";
	return result;
}

// RecordId alignment. align_by_record_id pairs the two spaces by shared record identity:
// the matched id list is taken in the left space's position order, and dropped_left /
// dropped_right report records that exist in only one space. This makes alignment explicit
// and auditable instead of assuming position i of one space pairs with position i of the
// other.
template <typename Value = double> struct AlignedCorrelationResult {
	using value_type = Value;

	CorrelationResult<Value> correlation;
	std::size_t left_record_count{};
	std::size_t right_record_count{};
	std::size_t matched_count{};
	std::size_t dropped_left{};
	std::size_t dropped_right{};
	std::vector<RecordId> matched_ids;
};

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto align_by_record_id(const LeftSpace &left_space, const RightSpace &right_space) -> std::vector<RecordId>
{
	std::vector<RecordId> matched;
	matched.reserve(left_space.size());
	for (std::size_t position = 0; position < left_space.size(); ++position) {
		const auto id = left_space.id(position);
		if (right_space.contains(id)) {
			matched.push_back(id);
		}
	}
	return matched;
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto mgc_by_record_id(const LeftSpace &left_space, const RightSpace &right_space) -> AlignedCorrelationResult<double>
{
	auto matched = align_by_record_id(left_space, right_space);

	AlignedCorrelationResult<double> result;
	result.left_record_count = left_space.size();
	result.right_record_count = right_space.size();
	result.matched_count = matched.size();
	result.dropped_left = left_space.size() - matched.size();
	result.dropped_right = right_space.size() - matched.size();
	result.matched_ids = matched;

	if (matched.size() < 2) {
		throw std::invalid_argument("MGC by RecordId requires at least two shared records");
	}
	detail::require_default_exact_mgc_records(matched.size(), "mgc_by_record_id(...)");

	std::vector<typename LeftSpace::record_type> left_records;
	std::vector<typename RightSpace::record_type> right_records;
	left_records.reserve(matched.size());
	right_records.reserve(matched.size());
	for (const auto id : matched) {
		left_records.push_back(left_space.record(id));
		right_records.push_back(right_space.record(id));
	}

	result.correlation = mgc(left_records, left_space.metric(), right_records, right_space.metric());
	result.correlation.left_representation = "metric_space_aligned_by_record_id";
	result.correlation.right_representation = "metric_space_aligned_by_record_id";
	return result;
}

} // namespace mtrc::stats::correlate

#endif
