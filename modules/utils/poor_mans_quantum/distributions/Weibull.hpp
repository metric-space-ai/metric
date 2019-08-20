/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_UTILS_POOR_MANS_QUANTUM_DISTRIBUTIONS_WEIBULL_HPP
#define _METRIC_UTILS_POOR_MANS_QUANTUM_DISTRIBUTIONS_WEIBULL_HPP

#include <vector>
#include <random>

namespace metric {
/**
 * @class Weibull
 * 
 * @brief class realize Weibull random distribution
 */
class Weibull {
public:
    /**
     * @brief Construct a new Weibull object
     * 
     */
    Weibull();

    /**
     * @brief sample a random value
     * 
     * @return random value according to Weibul distribution
     */

    float rnd();
    /**
     * @brief  long-run average value of repetitions of the same experiment
     * 
     * @return return the expected value of distribution
     */
    float mean();

    /**
     * @brief  quantil function
     * 
     * @param p 
     * @return value of the random variable such that the probability of the variable being less than or equal to that value equals the given probability.
     */
    float quantil(float p);

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
     * @brief  inverse cumulative distribution function 
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
}  // end namespace metric

#include "Weibull.cpp"
#endif  // header guard
