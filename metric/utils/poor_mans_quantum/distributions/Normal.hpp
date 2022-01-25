/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_UTILS_POOR_MANS_QUANTUM_DISTRIBUTIONS_NORMAL_HPP
#define _METRIC_UTILS_POOR_MANS_QUANTUM_DISTRIBUTIONS_NORMAL_HPP

#include <random>
#include <vector>

namespace metric {

/**
 * @class Normal
 * @brief class represent Normal distribution
 *
 */
class Normal {
  public:
	/**
	 * @brief Construct a new Normal object
	 *
	 */
	Normal();

	/*** sample a random value ***/

	/**
	 * @brief sample a random value
	 *
	 * @return random value according to Normal distribution
	 */
	float rnd();

	/**
	 * @brief
	 *
	 * @return
	 */
	float median();

	/**
	 * @brief  quantil function
	 *
	 * @param p
	 * @return value of the random variable such that the probability of the variable being less than or equal to that
	 * value equals the given probability.
	 */
	float quantil(float p);

	/**
	 * @brief long-run average value of repetitions of the same experiment
	 *
	 * @return return the expected value of distribution
	 */
	float mean();

	/**
	 * @brief  expectation of the squared deviation of a random variable from its mean
	 *
	 * @return variance value of the Normal distribution
	 */
	float variance();

	/**
	 * @brief probability density function
	 *
	 * @param x
	 * @return
	 */
	float pdf(const float x);

	/**
	 * @brief cumulative distribution function
	 *
	 * @param x
	 * @return
	 */
	float cdf(const float x);

	/**
	 * @brief inverse cumulative distribution function
	 *
	 * @param x
	 * @return
	 */
	float icdf(const float x);

	float _p1;
	float _p2;
	std::vector<float> _data;
	std::vector<float> _prob;

  private:
	std::mt19937_64 _generator;
};

} // end namespace metric
#include "Normal.cpp"
#endif // header guard
