// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_DISTANCE_MATRIX_HPP
#define _METRIC_REPRESENTATIONS_DISTANCE_MATRIX_HPP

#include <cstddef>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include <metric/numeric/Math.h>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>

namespace mtrc::space::storage {

inline constexpr std::size_t default_dense_distance_matrix_max_records = 4096;
inline constexpr std::size_t default_max_dense_distance_matrix_cells =
	default_dense_distance_matrix_max_records * default_dense_distance_matrix_max_records;

struct dense_distance_matrix_options {
	// Maximum number of cells that dense matrix conversion helpers may materialize.
	// Set to 0 only when the caller intentionally opts into an unbounded dense matrix.
	std::size_t max_cells{default_max_dense_distance_matrix_cells};
};

namespace distance_matrix_detail {

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

inline auto require_dense_matrix_budget(std::size_t record_count, dense_distance_matrix_options options,
										const char *operation) -> std::size_t
{
	if (record_count != 0 && record_count > std::numeric_limits<std::size_t>::max() / record_count) {
		throw RepresentationError(std::string(operation) + " cannot represent record_count * record_count");
	}
	const auto cell_count = record_count * record_count;
	if (options.max_cells == 0 || cell_count <= options.max_cells) {
		return cell_count;
	}
	throw RepresentationError(std::string(operation) +
							  " refused to materialize a dense distance matrix before allocation: records=" +
							  std::to_string(record_count) + ", cells=" + std::to_string(cell_count) +
							  ", max_cells=" + std::to_string(options.max_cells) +
							  ". Use lazy/streaming providers for large spaces or pass dense_distance_matrix_options{0} "
							  "only when an unbounded dense matrix is intentional.");
}

} // namespace distance_matrix_detail

template <typename Scalar, typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto provider_dense_distance_matrix(const Provider &provider, dense_distance_matrix_options options = {})
	-> numeric::DynamicMatrix<Scalar>
{
	(void)distance_matrix_detail::require_dense_matrix_budget(provider.record_count(), options,
															  "provider_dense_distance_matrix");
	numeric::DynamicMatrix<Scalar> matrix(provider.record_count(), provider.record_count(), Scalar(0));
	for (std::size_t row = 0; row < provider.record_count(); ++row) {
		for (std::size_t column = 0; column < provider.record_count(); ++column) {
			matrix(row, column) =
				static_cast<Scalar>(distance_matrix_detail::distance_at_pair_position(provider, row, column));
		}
	}
	return matrix;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto provider_dense_distance_matrix(const Provider &provider, dense_distance_matrix_options options = {})
	-> numeric::DynamicMatrix<typename Provider::distance_type>
{
	return provider_dense_distance_matrix<typename Provider::distance_type>(provider, options);
}

template <typename Scalar, typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto provider_symmetric_distance_matrix(const Provider &provider, dense_distance_matrix_options options = {})
	-> numeric::SymmetricMatrix<numeric::DynamicMatrix<Scalar>>
{
	(void)distance_matrix_detail::require_dense_matrix_budget(provider.record_count(), options,
															  "provider_symmetric_distance_matrix");
	numeric::SymmetricMatrix<numeric::DynamicMatrix<Scalar>> matrix(provider.record_count());
	for (std::size_t row = 0; row < provider.record_count(); ++row) {
		matrix(row, row) = Scalar(0);
		for (std::size_t column = row + 1; column < provider.record_count(); ++column) {
			matrix(row, column) =
				static_cast<Scalar>(distance_matrix_detail::distance_at_pair_position(provider, row, column));
		}
	}
	return matrix;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto provider_symmetric_distance_matrix(const Provider &provider, dense_distance_matrix_options options = {})
	-> numeric::SymmetricMatrix<numeric::DynamicMatrix<typename Provider::distance_type>>
{
	return provider_symmetric_distance_matrix<typename Provider::distance_type>(provider, options);
}

template <typename Scalar, typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto metric_space_dense_distance_matrix(const Space &space, dense_distance_matrix_options options = {})
	-> numeric::DynamicMatrix<Scalar>
{
	(void)distance_matrix_detail::require_dense_matrix_budget(space.size(), options,
															  "metric_space_dense_distance_matrix");
	numeric::DynamicMatrix<Scalar> matrix(space.size(), space.size(), Scalar(0));
	for (std::size_t row = 0; row < space.size(); ++row) {
		for (std::size_t column = 0; column < space.size(); ++column) {
			matrix(row, column) = static_cast<Scalar>(space.distance(space.id(row), space.id(column)));
		}
	}
	return matrix;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto metric_space_dense_distance_matrix(const Space &space, dense_distance_matrix_options options = {})
	-> numeric::DynamicMatrix<typename Space::distance_type>
{
	return metric_space_dense_distance_matrix<typename Space::distance_type>(space, options);
}

} // namespace mtrc::space::storage

#endif
