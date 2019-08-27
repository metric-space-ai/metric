/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_MAPPING_SOM_HPP
#define _METRIC_MAPPING_SOM_HPP

/*
for specific setting check:
Appropriate Learning Rate and Neighborhood Function of Self-organizing Map (SOM) ...
International Journal of Modeling and Optimization, Vol. 6, No. 1, February 2016
W. Natita, W. Wiboonsak, and S. Dusadee
*/

#include <assert.h>

#include <iostream>
#include <array>
#include <vector>

#include <chrono>
#include <random>
#include <algorithm>
#include <cmath>
#include <numeric>

//#include "metric.tpp"
#include "../distance.hpp"
#include "../utils/graph.hpp"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

namespace metric {
/**
	 * @class SOM
	 * 
	 *@brief 
	 * 
	 */
template <typename recType, class Metric = metric::Euclidian<typename recType::value_type>, class Graph = metric::Grid8, class Distribution = std::uniform_real_distribution<typename recType::value_type>>
class SOM {
    typedef typename recType::value_type T;

public:
    /**
	     * @brief Construct a new SOM object
	     * 
	     * @param nodesNumber 
	     * @param metric 
	     */
    explicit SOM(size_t nodesNumber, Metric metric = Metric());

    /**
		 * @brief Construct a new SOM object
		 * 
		 * @param nodesWidth 
		 * @param nodesHeight 
		 * @param metric 
		 */
    SOM(size_t nodesWidth, size_t nodesHeight, Metric metric = Metric());

    /**
	     * @brief Construct a new SOM object
	     * 
	     * @param metric 
	     * @param graph 
	     * @param distribution 
	     */
    SOM(Metric metric, Graph graph, Distribution distribution, double neighborhoodSize, double neigbour_range_decay, long long random_seed);

    /**
		 * @brief Destroy the SOM object
		 * 
		 */
    ~SOM() = default;
    /**
		 * @brief 
		 * 
		 * @return true 
		 * @return false 
		 */
    bool isValid();

    /**
		 * @brief 
		 * 
		 * @param samples 
		 * @param iterations 
		 * @param s_learn_rate 
		 * @param f_learn_rate 
		 */
    void train(const std::vector<std::vector<T>>& samples, size_t iterations = 10000, double s_learn_rate = 1.2,
        double f_learn_rate = 0.4);

    /**
		 * @brief 
		 * 
		 * @param sample 
		 * @return
		 */
    std::vector<double> reduce(const recType& sample);

    /**
		 * @brief 
		 * 
		 * @return
		 */
    size_t size();

    /**
		 * @brief Best matching unit
		 * 
		 * @param sample 
		 * @return size_t 
		 */
    size_t BMU(const recType& sample) const;

private:
    bool valid;
    size_t D;  // dimensions of inputs vector

    Metric metric;
    Graph graph;
    Distribution distribution;

	double neighborhoodSize;
	double neigbour_range_decay;
	long long random_seed;

    std::vector<std::vector<T>> weights;  // nodes of SOM
};

}  // namespace metric
#include "SOM.cpp"
#endif
