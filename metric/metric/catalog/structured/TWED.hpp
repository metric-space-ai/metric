/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/
#ifndef _METRIC_DISTANCE_K_RANDOM_TWED_HPP
#define _METRIC_DISTANCE_K_RANDOM_TWED_HPP

#include <cmath>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <metric/core/metric_traits.hpp>
#include <metric/numeric/Math.h>

namespace mtrc {

/**
 * @class TWED
 *
 * @brief Time Warp Edit Distance for finite real-valued sequences (Marteau, 2009).
 *
 * Admitted metric domain
 * ----------------------
 * The admitted metric record is a finite, non-empty, densely indexed sequence
 * of finite real values `x_0, x_1, ..., x_{n-1}` read positionally, so the
 * timestamp of position `i` is `i`. Two records are equal when they have the
 * same length and the same value at every position. This is the domain that
 * the property tests cover and that the `metric_law::metric` trait describes.
 *
 * The same recurrence accepts a `mtrc::numeric::CompressedVector`, using the
 * stored index of each non-zero element as its timestamp. It is a metric on the
 * resulting `(value, timestamp)` sequences, but the sparse-to-sequence mapping
 * drops zero entries and ignores trailing zeros, so two distinct compressed
 * records can map to the same sequence and collapse the identity of
 * indiscernibles. The CompressedVector path is therefore a compatibility input
 * route, not part of the admitted metric record domain.
 *
 * Parameter constraints
 * ---------------------
 * - `elastic` (the stiffness `nu`) must be finite and strictly positive
 *   (`elastic > 0`). It scales the temporal cost and is what separates
 *   sequences that differ only by timestamp/length, so it is required for the
 *   identity of indiscernibles.
 * - `penalty` (the deletion penalty `lambda`) must be finite and non-negative
 *   (`penalty >= 0`).
 *
 * Under these constraints TWED is non-negative, symmetric, zero exactly on
 * equal records, and satisfies the triangle inequality on the dense positional
 * domain, so it is admitted as `metric_law::metric`. Both the constructor and
 * every distance evaluation reject parameters outside the admitted domain, and
 * every evaluation rejects empty or non-finite sequences. See
 * docs/metrics/metric-admission.md for the admission record and
 * docs/metrics/true-metric-catalog.md for the remaining proof gap.
 */
template <typename V> struct TWED {
	using value_type = V;
	using distance_type = value_type;

	/**
	 * @brief Construct a new TWED object
	 *
	 * @param penalty_ deletion penalty `lambda`, must be finite and `>= 0`
	 * @param elastic_ temporal stiffness `nu`, must be finite and `> 0`
	 * @throws std::invalid_argument if the parameters leave the admitted metric domain
	 */
	TWED(const value_type &penalty_ = 0, const value_type &elastic_ = 1) : penalty(penalty_), elastic(elastic_)
	{
		validate_parameters();
	}

	/**
	 * @brief Calculate TWE distance between given containers
	 *
	 * @param As first container
	 * @param Bs second container
	 * @return TWE distance between given containers
	 * @throws std::invalid_argument on invalid parameters, empty inputs, or non-finite values
	 */
	template <typename Container> value_type operator()(const Container &As, const Container &Bs) const;

	value_type penalty = 0;
	value_type elastic = 1;
	bool is_zero_padded = false;

  private:
	/// Enforce the admitted parameter gate (penalty >= 0, elastic > 0, both finite).
	void validate_parameters() const
	{
		if constexpr (std::is_floating_point<value_type>::value) {
			if (!std::isfinite(penalty) || !std::isfinite(elastic)) {
				throw std::invalid_argument("TWED requires finite penalty and elastic parameters");
			}
		}
		if (penalty < value_type(0)) {
			throw std::invalid_argument("TWED requires a finite nonnegative penalty (penalty >= 0)");
		}
		if (elastic <= value_type(0)) {
			throw std::invalid_argument("TWED requires a finite positive elastic stiffness (elastic > 0)");
		}
	}
};

} // namespace mtrc

namespace mtrc::core {

template <typename V> struct metric_traits<::mtrc::TWED<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::sequence;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::mtrc::TWED<V> &metric) -> std::string
	{
		return std::string("mtrc::TWED:penalty=") + std::to_string(metric.penalty) +
			   ":elastic=" + std::to_string(metric.elastic) + ":zero_padded=" + (metric.is_zero_padded ? "1" : "0");
	}
};

} // namespace mtrc::core

#include "TWED.cpp"

#endif // Header Guard
