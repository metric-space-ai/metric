/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_DISTANCE_K_RELATED_L1_HPP
#define _METRIC_DISTANCE_K_RELATED_L1_HPP

#include <metric/numeric.hpp>

#include <type_traits>

#include <metric/core/metric_traits.hpp>
#include <metric/metric/detail/vector_input.hpp>

namespace mtrc {

template <typename V = double> class Sorensen {
  public:
	using distance_type = V;

	explicit Sorensen() = default;

	/**
	 * @brief
	 *
	 * @param a
	 * @param b
	 * @return
	 */
	template <typename Container>
	typename std::enable_if<!std::is_same<Container, V>::value, distance_type>::type
	operator()(const Container &a, const Container &b) const;

	/**
	 * @brief
	 *
	 * @param a
	 * @param b
	 * @return
	 */
	distance_type operator()(const mtrc::numeric::CompressedVector<V> &a,
							 const mtrc::numeric::CompressedVector<V> &b) const;

	// TODO add support of 1D random values passed in simple containers
};

template <typename V = double> struct Hassanat {
	using value_type = V;
	using distance_type = value_type;

	explicit Hassanat() = default;

	/**
	 * @brief Calculate Hassanat distance in R^n
	 *
	 * @param a first vector
	 * @param b second vector
	 * @return Hassanat distance between a and b
	 */

	template <typename Container> distance_type operator()(const Container &a, const Container &b) const;
};

template <typename Value>
double sorensen(const mtrc::numeric::CompressedVector<Value> &a, const mtrc::numeric::CompressedVector<Value> &b);

template <typename V = double> struct Ruzicka {
	using value_type = V;
	using distance_type = value_type;

	explicit Ruzicka() = default;

	/**
	 * @brief Calculate Ruzicka distance in R^n
	 *
	 * @param a first vector
	 * @param b second vector
	 * @return Ruzicka distance between a and b
	 */

	template <typename Container> distance_type operator()(const Container &a, const Container &b) const;
};

} // namespace mtrc

namespace mtrc::core {

template <typename V> struct metric_traits<::mtrc::Ruzicka<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::aligned_vector;
	static constexpr bool thread_safe = true;
};

} // namespace mtrc::core

#include "L1.cpp"

#endif // Header Guard
