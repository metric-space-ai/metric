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
template <typename recType, class Graph = metric::Grid6, class Metric = metric::Euclidian<typename recType::value_type>, class Distribution = std::uniform_real_distribution<typename recType::value_type>>
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
	     * @param graph 
	     * @param metric 
		 * @param s_learn_rate 
		 * @param f_learn_rate 
		 * @param iterations 
	     * @param distribution 
	     */
    SOM(Graph graph, Metric metric = Metric(), double start_learn_rate = 0.8, double finish_learn_rate = 0.0, size_t iterations = 20, 
		Distribution distribution = Distribution(-1, 1));

    /**
	     * @brief Construct a new SOM object
	     * 
	     * @param graph 
	     * @param metric 
		 * @param s_learn_rate 
		 * @param f_learn_rate 
		 * @param iterations 
	     * @param distribution 
		 * @param neighborhood_start_size 
		 * @param neigbour_range_decay 
		 * @param random_seed 
	     */
    SOM(Graph graph, Metric metric, double start_learn_rate, double finish_learn_rate, size_t iterations, 
		Distribution distribution, double neighborhood_start_size, double neigbour_range_decay, long long random_seed);

    /**
		 * @brief Destroy the SOM object
		 * 
		 */
    ~SOM() = default;

    /**
		 * @brief 
		 * 
		 * @param samples 
		 */
    virtual void train(const std::vector<std::vector<T>>& samples);

    /**
		 * @brief 
		 * 
		 * @param samples 
		 */
    virtual void estimate(const std::vector<std::vector<T>>& samples, const size_t sampleSize);

    /**
		 * @brief 
		 * 
		 * @param sample 
		 * @return
		 */
    virtual std::vector<double> encode(const recType& sample);

    /**
		 * @brief Best matching unit
		 * 
		 * @param sample 
		 * @return size_t 
		 */
    virtual size_t BMU(const recType& sample) const;

    /**
		 * @brief 
		 * 
		 * @return true 
		 * @return false 
		 */
	bool isValid()
	{
		return valid;
	}

    /**
		 * @brief 
		 * 
		 * @param samples 
		 * 
		 * @return double 
		 */
	double std_deviation(const std::vector<std::vector<T>>& samples);

    /**
		 * @brief 
		 * 
		 * @param sample_1
		 * @param sample_2
		 * 
		 * @return T
		 */
	double kohonen_distance(const std::vector<T>& sample_1, const std::vector<T>& sample_2);

    /**
		 * @brief 
		 * 
		 * @return
		 */
    size_t getNodesNumber()
	{
		return graph.getNodesNumber();
	}

    /**
		 * @brief
		 * 
		 * @return
		 */
	std::vector<std::vector<T>> get_weights()
	{
		return weights;
	}

protected:
    size_t input_dimensions;  // dimensions of inputs vector

    Metric metric;
    Graph graph;
    Distribution distribution;

	double start_learn_rate;
	double finish_learn_rate; 
	size_t iterations;

	double neighborhood_start_size;
	double neigbour_range_decay;
	long long random_seed;
	
    /**
		 * @brief 
		 * 
		 * @param samples 
		 */
    void subsampled_train_(const std::vector<std::vector<T>>& samples, int sampleSize);
	
private:
    bool valid;
    std::vector<std::vector<T>> weights;  // coordinates in the input_dimensions space 
};

}  // namespace metric
#include "SOM.cpp"
#endif
