// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_CORRELATION_HPP
#define _METRIC_OPERATORS_CORRELATION_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <metric/core/errors.hpp>
#include <metric/core/result.hpp>
#include <metric/correlation/mgc.hpp>
#include <metric/space/storage/distance_matrix.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/stats/correlate/options.hpp>

namespace mtrc::stats::correlate {

namespace detail {

inline constexpr std::size_t max_default_exact_mgc_records = 4096;
inline constexpr std::size_t max_default_exact_mgc_scratch_records = 2886;

inline auto require_default_exact_mgc_records(std::size_t record_count, const char *operation) -> void
{
	if (record_count <= max_default_exact_mgc_records) {
		return;
	}
	throw RepresentationError(
		std::string(operation) + " refused exact MGC before metric calls: records=" +
		std::to_string(record_count) +
		", max_exact_records=" + std::to_string(max_default_exact_mgc_records) +
		". Use compare(...), correlate(...), or mgc_estimate(...) with a bounded sample_count for large spaces.");
}

inline auto require_default_exact_mgc_scratch_records(std::size_t record_count, const char *operation) -> void
{
	if (record_count <= max_default_exact_mgc_scratch_records) {
		return;
	}
	throw RepresentationError(
		std::string(operation) + " refused exact MGC scratch work before metric calls: records=" +
		std::to_string(record_count) +
		", max_scratch_records=" + std::to_string(max_default_exact_mgc_scratch_records) +
		". Use compare(...), correlate(...), or mgc_estimate(...) with a bounded sample_count for large spaces.");
}

inline auto effective_mgc_sample_count(std::size_t record_count, mgc_options options) -> std::size_t
{
	if (options.sample_count == 0) {
		throw std::invalid_argument("MGC estimate sample_count must be >= 1");
	}
	return options.sample_count > record_count ? record_count : options.sample_count;
}

inline auto effective_mgc_iterations(std::size_t record_count, std::size_t sample_count, mgc_options options)
	-> std::size_t
{
	if (sample_count == 0 || sample_count >= record_count) {
		return 1;
	}
	const auto maximum_full_samples = record_count / sample_count;
	if (maximum_full_samples == 0) {
		return 1;
	}
	if (options.max_iterations == 0 || options.max_iterations > maximum_full_samples) {
		return maximum_full_samples;
	}
	return options.max_iterations;
}

} // namespace detail

// Multiscale Graph Correlation (MGC) answers the Level-1 question "are two paired
// finite metric spaces dependent?". It is a *dependence statistic*, NOT a metric:
//
//   * The returned value is a single sample MGC statistic in the range [-1, 1].
//     ~1 indicates strong (possibly nonlinear / monotone) dependence, ~0 indicates
//     no detected dependence. A constant (zero-distance) space yields 0.
//   * MGC is symmetric in practice (mgc(X, Y) == mgc(Y, X) for the tested fixtures),
//     but exact symmetry is an observed property of the implementation, not a
//     guarantee derived from the algorithm's region-selection step.
//   * No p-value or permutation/significance test is computed here. CorrelationResult
//     carries only the statistic; significance testing is a separate concern.
//   * Both spaces must contain the same number of paired records, and at least two
//     records, so the centered/ranked distance matrices are well defined. Very small
//     samples (n <= 4) make the significant-region heuristic degenerate and the
//     statistic falls back to the maximal-scale correlation; treat such results as
//     weak. See docs/examples/correlation-between-spaces.md.
template <typename LeftContainer, typename LeftMetric, typename RightContainer, typename RightMetric,
		  typename LeftRecord = typename std::decay<typename LeftContainer::value_type>::type,
		  typename RightRecord = typename std::decay<typename RightContainer::value_type>::type,
		  typename std::enable_if<
			  MetricCallable_v<LeftMetric, LeftRecord> && MetricCallable_v<RightMetric, RightRecord>, int>::type = 0>
auto mgc(const LeftContainer &left_records, const LeftMetric &left_metric, const RightContainer &right_records,
		 const RightMetric &right_metric) -> CorrelationResult<double>
{
	if (left_records.size() != right_records.size()) {
		throw std::invalid_argument("MGC requires spaces with the same record count");
	}
	if (left_records.size() < 2) {
		throw std::invalid_argument("MGC requires at least two paired records");
	}
	detail::require_default_exact_mgc_records(left_records.size(), "mgc(...)");
	detail::require_default_exact_mgc_scratch_records(left_records.size(), "mgc(...)");

	mtrc::MGC<LeftRecord, LeftMetric, RightRecord, RightMetric> estimator(left_metric, right_metric);

	return core::make_correlation_result(estimator(left_records, right_records), left_records.size(),
										 right_records.size(), "mgc", "records", "records");
}

template <typename LeftContainer, typename LeftMetric, typename RightContainer, typename RightMetric,
		  typename LeftRecord = typename std::decay<typename LeftContainer::value_type>::type,
		  typename RightRecord = typename std::decay<typename RightContainer::value_type>::type,
		  typename std::enable_if<
			  MetricCallable_v<LeftMetric, LeftRecord> && MetricCallable_v<RightMetric, RightRecord>, int>::type = 0>
auto mgc_estimate(const LeftContainer &left_records, const LeftMetric &left_metric, const RightContainer &right_records,
				  const RightMetric &right_metric, mgc_options options) -> CorrelationResult<double>
{
	if (left_records.size() != right_records.size()) {
		throw std::invalid_argument("MGC estimate requires spaces with the same record count");
	}
	if (left_records.size() < 2) {
		throw std::invalid_argument("MGC estimate requires at least two paired records");
	}

	mtrc::MGC<LeftRecord, LeftMetric, RightRecord, RightMetric> estimator(left_metric, right_metric);
	const auto sample_count = detail::effective_mgc_sample_count(left_records.size(), options);
	const auto sample_iterations = detail::effective_mgc_iterations(left_records.size(), sample_count, options);

	if (sample_count >= left_records.size()) {
		detail::require_default_exact_mgc_records(left_records.size(), "mgc_estimate(...)");
		detail::require_default_exact_mgc_scratch_records(left_records.size(), "mgc_estimate(...)");
		auto result = core::make_correlation_result(estimator(left_records, right_records), left_records.size(),
												 right_records.size(), "mgc", "records", "records", true);
		result.sample_count = sample_count;
		result.sample_iterations = 1;
		result.approximation_reason = "sample covers all records; exact MGC was used";
		return result;
	}

	auto result = core::make_correlation_result(
		estimator.estimate(left_records, right_records, sample_count, options.estimate_threshold, sample_iterations),
		left_records.size(), right_records.size(), "mgc_estimate", "records_sample", "records_sample", false);
	result.sample_count = sample_count;
	result.sample_iterations = sample_iterations;
	result.approximation_reason = "approximate runtime policy selected subsampled MGC";
	return result;
}

template <typename LeftProvider, typename RightProvider,
		  typename std::enable_if<PairwiseDistances_v<LeftProvider> && PairwiseDistances_v<RightProvider>, int>::type = 0>
auto mgc(const LeftProvider &left_provider, const RightProvider &right_provider) -> CorrelationResult<double>
{
	if (left_provider.record_count() != right_provider.record_count()) {
		throw std::invalid_argument("MGC requires spaces with the same record count");
	}
	if (left_provider.record_count() < 2) {
		throw std::invalid_argument("MGC requires at least two paired records");
	}
	detail::require_default_exact_mgc_scratch_records(left_provider.record_count(), "mgc(provider, provider)");

	auto left_distances = space::storage::provider_symmetric_distance_matrix<double>(left_provider);
	auto right_distances = space::storage::provider_symmetric_distance_matrix<double>(right_provider);

	return core::make_correlation_result(MGC_direct()(left_distances, right_distances), left_provider.record_count(),
										 right_provider.record_count(), "mgc", "pairwise_distances",
										 "pairwise_distances");
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto mgc(const LeftSpace &left_space, const RightSpace &right_space) -> CorrelationResult<double>
{
	if (left_space.size() != right_space.size()) {
		throw std::invalid_argument("MGC requires spaces with the same record count");
	}
	if (left_space.size() < 2) {
		throw std::invalid_argument("MGC requires at least two paired records");
	}
	detail::require_default_exact_mgc_records(left_space.size(), "mgc(metric_space, metric_space)");
	detail::require_default_exact_mgc_scratch_records(left_space.size(), "mgc(metric_space, metric_space)");

	space::storage::LiveDistances<LeftSpace> left_provider(left_space);
	space::storage::LiveDistances<RightSpace> right_provider(right_space);
	auto result = mgc(left_provider, right_provider);
	result.left_representation = "metric_space";
	result.right_representation = "metric_space";
	return result;
}

template <typename LeftSpace, typename RightSpace,
		  typename std::enable_if<MetricSpaceLike_v<LeftSpace> && MetricSpaceLike_v<RightSpace>, int>::type = 0>
auto mgc_estimate(const LeftSpace &left_space, const RightSpace &right_space, mgc_options options)
	-> CorrelationResult<double>
{
	auto result =
		mgc_estimate(left_space.records(), left_space.metric(), right_space.records(), right_space.metric(), options);
	if (result.exact) {
		result.left_representation = "metric_space";
		result.right_representation = "metric_space";
	} else {
		result.left_representation = "metric_space_sample";
		result.right_representation = "metric_space_sample";
	}
	return result;
}

} // namespace mtrc::stats::correlate

#endif
