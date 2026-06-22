// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_CORRELATION_HPP
#define _METRIC_OPERATORS_CORRELATION_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>

#include <metric/core/result.hpp>
#include <metric/correlation/mgc.hpp>
#include <metric/space/storage/distance_matrix.hpp>

namespace mtrc::stats::correlate {

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

	mtrc::MGC<LeftRecord, LeftMetric, RightRecord, RightMetric> estimator(left_metric, right_metric);

	return core::make_correlation_result(estimator(left_records, right_records), left_records.size(),
										 right_records.size(), "mgc", "records", "records");
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
	auto result = mgc(left_space.records(), left_space.metric(), right_space.records(), right_space.metric());
	result.left_representation = "metric_space";
	result.right_representation = "metric_space";
	return result;
}

} // namespace mtrc::stats::correlate

#endif
