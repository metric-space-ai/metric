// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_HPP
#define _METRIC_OPERATORS_HPP

#include "space.hpp"

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

} // namespace metric::operators

#endif
