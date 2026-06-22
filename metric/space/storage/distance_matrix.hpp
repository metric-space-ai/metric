// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_DISTANCE_MATRIX_HPP
#define _METRIC_REPRESENTATIONS_DISTANCE_MATRIX_HPP

#include <cstddef>
#include <type_traits>

#include <metric/numeric/Math.h>

#include <metric/core/concepts.hpp>

namespace mtrc::space::storage {

template <typename Scalar, typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto provider_dense_distance_matrix(const Provider &provider) -> numeric::DynamicMatrix<Scalar>
{
	numeric::DynamicMatrix<Scalar> matrix(provider.record_count(), provider.record_count(), Scalar(0));
	for (std::size_t row = 0; row < provider.record_count(); ++row) {
		for (std::size_t column = 0; column < provider.record_count(); ++column) {
			matrix(row, column) = static_cast<Scalar>(provider.distance(provider.id(row), provider.id(column)));
		}
	}
	return matrix;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto provider_dense_distance_matrix(const Provider &provider)
	-> numeric::DynamicMatrix<typename Provider::distance_type>
{
	return provider_dense_distance_matrix<typename Provider::distance_type>(provider);
}

template <typename Scalar, typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto provider_symmetric_distance_matrix(const Provider &provider)
	-> numeric::SymmetricMatrix<numeric::DynamicMatrix<Scalar>>
{
	numeric::SymmetricMatrix<numeric::DynamicMatrix<Scalar>> matrix(provider.record_count());
	for (std::size_t row = 0; row < provider.record_count(); ++row) {
		matrix(row, row) = Scalar(0);
		for (std::size_t column = row + 1; column < provider.record_count(); ++column) {
			matrix(row, column) = static_cast<Scalar>(provider.distance(provider.id(row), provider.id(column)));
		}
	}
	return matrix;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto provider_symmetric_distance_matrix(const Provider &provider)
	-> numeric::SymmetricMatrix<numeric::DynamicMatrix<typename Provider::distance_type>>
{
	return provider_symmetric_distance_matrix<typename Provider::distance_type>(provider);
}

template <typename Scalar, typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto metric_space_dense_distance_matrix(const Space &space) -> numeric::DynamicMatrix<Scalar>
{
	numeric::DynamicMatrix<Scalar> matrix(space.size(), space.size(), Scalar(0));
	for (std::size_t row = 0; row < space.size(); ++row) {
		for (std::size_t column = 0; column < space.size(); ++column) {
			matrix(row, column) = static_cast<Scalar>(space.distance(space.id(row), space.id(column)));
		}
	}
	return matrix;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto metric_space_dense_distance_matrix(const Space &space) -> numeric::DynamicMatrix<typename Space::distance_type>
{
	return metric_space_dense_distance_matrix<typename Space::distance_type>(space);
}

} // namespace mtrc::space::storage

#endif
