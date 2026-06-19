// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_CORRELATION_HPP
#define _METRIC_OPERATORS_CORRELATION_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>

#include "../core/concepts.hpp"
#include "../core/result.hpp"
#include "../correlation/mgc.hpp"

namespace metric::operators {
namespace detail {

template <typename Provider, typename std::enable_if<DistanceProvider_v<Provider>, int>::type = 0>
auto provider_distance_matrix(const Provider &provider) -> DistanceMatrix<double>
{
	DistanceMatrix<double> matrix(provider.record_count());
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		matrix(row, row) = 0.0;
		for (std::size_t column = row + 1; column < matrix.columns(); ++column) {
			matrix(row, column) = static_cast<double>(provider.distance(provider.id(row), provider.id(column)));
		}
	}
	return matrix;
}

} // namespace detail

template <typename LeftContainer, typename LeftMetric, typename RightContainer, typename RightMetric,
		  typename LeftRecord = typename std::decay<typename LeftContainer::value_type>::type,
		  typename RightRecord = typename std::decay<typename RightContainer::value_type>::type,
		  typename std::enable_if<MetricCallable_v<LeftMetric, LeftRecord> &&
									  MetricCallable_v<RightMetric, RightRecord>,
								  int>::type = 0>
auto mgc(const LeftContainer &left_records, const LeftMetric &left_metric, const RightContainer &right_records,
		 const RightMetric &right_metric) -> CorrelationResult<double>
{
	if (left_records.size() != right_records.size()) {
		throw std::invalid_argument("MGC requires spaces with the same record count");
	}

	metric::MGC<LeftRecord, LeftMetric, RightRecord, RightMetric> estimator(left_metric, right_metric);

	CorrelationResult<double> result;
	result.value = estimator(left_records, right_records);
	result.left_record_count = left_records.size();
	result.right_record_count = right_records.size();
	result.exact = true;
	result.algorithm = "mgc";
	result.left_representation = "records";
	result.right_representation = "records";
	return result;
}

template <typename LeftProvider, typename RightProvider,
		  typename std::enable_if<DistanceProvider_v<LeftProvider> && DistanceProvider_v<RightProvider>, int>::type = 0>
auto mgc(const LeftProvider &left_provider, const RightProvider &right_provider) -> CorrelationResult<double>
{
	if (left_provider.record_count() != right_provider.record_count()) {
		throw std::invalid_argument("MGC requires spaces with the same record count");
	}

	auto left_distances = detail::provider_distance_matrix(left_provider);
	auto right_distances = detail::provider_distance_matrix(right_provider);

	CorrelationResult<double> result;
	result.value = MGC_direct()(left_distances, right_distances);
	result.left_record_count = left_provider.record_count();
	result.right_record_count = right_provider.record_count();
	result.exact = true;
	result.algorithm = "mgc";
	result.left_representation = "distance_provider";
	result.right_representation = "distance_provider";
	return result;
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

} // namespace metric::operators

#endif
