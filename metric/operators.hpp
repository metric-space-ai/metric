// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_HPP
#define _METRIC_OPERATORS_HPP

#include "space.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

namespace metric::operators {

namespace detail {

template <typename Container> using record_type_t = typename std::decay<typename Container::value_type>::type;

template <typename Container, typename Metric>
using finite_space_t = ::metric::FiniteSpace<record_type_t<Container>, Metric>;

} // namespace detail

template <typename Container, typename Metric>
auto pairwise_distance_matrix(const Container &records, Metric distance)
	-> std::vector<std::vector<typename detail::finite_space_t<Container, Metric>::distance_type>>
{
	return ::metric::Space::from_records(records, std::move(distance)).pairwise_distances();
}

template <typename Container, typename Metric>
auto nearest_neighbors(const Container &records, Metric distance, const detail::record_type_t<Container> &query,
					   unsigned k = 1)
{
	return ::metric::Space::from_records(records, std::move(distance)).neighbors(query, k);
}

template <typename Container, typename Metric>
auto range_neighbors(const Container &records, Metric distance, const detail::record_type_t<Container> &query,
					 typename detail::finite_space_t<Container, Metric>::distance_type radius)
{
	return ::metric::Space::from_records(records, std::move(distance)).within_radius(query, radius);
}

template <typename Container, typename Metric>
auto intrinsic_dimension(const Container &records, Metric distance) -> double
{
	using distance_type = typename detail::finite_space_t<Container, Metric>::distance_type;

	const auto distances = pairwise_distance_matrix(records, std::move(distance));

	double maximum_dimension = 0.0;
	for (const auto &row : distances) {
		for (const auto &radius : row) {
			if (radius <= distance_type{}) {
				continue;
			}

			const auto outer_radius = radius + radius;
			const auto inner_count = static_cast<double>(
				std::count_if(row.begin(), row.end(), [&](const auto &value) { return value <= radius; }));
			const auto outer_count = static_cast<double>(
				std::count_if(row.begin(), row.end(), [&](const auto &value) { return value <= outer_radius; }));

			if (inner_count > 0.0 && outer_count >= inner_count) {
				maximum_dimension = std::max(maximum_dimension, std::log(outer_count / inner_count) / std::log(2.0));
			}
		}
	}

	return maximum_dimension;
}

} // namespace metric::operators

#endif
