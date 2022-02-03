/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
  Copyright (c) 2019 Michael Welsch
*/
#ifndef _METRIC_UTILS_POOR_MANS_QUANTUM_DISTRIBUTIONS_BINOMIAL_HPP
#define _METRIC_UTILS_POOR_MANS_QUANTUM_DISTRIBUTIONS_BINOMIAL_HPP

#include <random>
#include <vector>

namespace metric {
/**
 * @class Binomial
 *
 * @brief Class represents binomial distribution
 */

class Binomial {
  public:
	/**
	 * @brief Construct a new Binomial object
	 *
	 */
	Binomial();

	/**
	 * @brief sample a random value
	 *
	 * @return random value according to Normal distribution
	 */
	float rnd();

	/**
	 * @brief cumulative distribution function
	 *
	 * @param x
	 * @return
	 */
	float cdf(const int x);

	// float icdf(const float x);
	// float mean();
	// float quantil(float p);

	// float pdf(const float x);

	float _p1;
	float _p2;
	std::vector<float> _data;
	std::vector<float> _prob;

  private:
	std::mt19937_64 _generator;
};

} // namespace metric
#include "Binomial.cpp"
#endif // header guard