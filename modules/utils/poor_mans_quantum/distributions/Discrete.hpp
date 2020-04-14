/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/
#ifndef _METRIC_UTILS_POOR_MANS_QUANTUM_DISTRIBUTIONS_DISCRETE_HPP
#define _METRIC_UTILS_POOR_MANS_QUANTUM_DISTRIBUTIONS_DISCRETE_HPP

#include <random>
#include <vector>
namespace metric {

/**
 * @class Discrete
 * 
 * @brief Class represents discrete distribution
 */
template <typename T = float>
class Discrete {
public:
    /**
     * @brief Construct a new Discrete object
     * 
     */
    Discrete();

    /**
     * @brief 
     * 
     * @param seed 
     */
    void seed(size_t seed);

    /**
     * @brief sample a random value
     * 
     * @return random value according to Normal distribution
     */
    T rnd();

    /**
     * @brief 
     * 
     * @return
     */
    T median();

    /**
     * @brief  quantil function
     * 
     * @param p 
     * @return value of the random variable such that the probability of the variable being less than or equal to that value equals the given probability.
     */
    T quantil(T p);

    /**
     * @brief long-run average value of repetitions of the same experiment
     * 
     * @return return the expected value of distribution
     */
    T mean();

    /**
     * @brief  expectation of the squared deviation of a random variable from its mean
     * 
     * @return variance value of the Normal distribution
     */
    T variance();

    /**
     * @brief probability density function 
     * 
     * @param x 
     * @return
     */
    T pdf(const T x);

    /**
     * @brief cumulative distribution function 
     * 
     * @param x 
     * @return
     */
    T cdf(const T x);

    /**
     * @brief  inverse cumulative distribution function 
     * 
     * @param x 
     * @return
     */
    T icdf(const T x);

    T _p1;
    T _p2;
    std::vector<T> _data;
    std::vector<T> _prob;

private:
    std::mt19937_64 _generator;
};

}  // end namespace metric
#include "Discrete.cpp"
#endif  // header guard
