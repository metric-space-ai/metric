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

#include "metric/distance/k-related/Standards.hpp"
#include "metric/utils/graph.hpp"

#include <assert.h>

#include <array>
#include <iostream>
#include <vector>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <numeric>
#include <random>

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
template <typename RecType, class Graph = metric::Grid6, class Metric = metric::Euclidean<typename RecType::value_type>,
		  class Distribution = std::uniform_real_distribution<typename RecType::value_type>>
class SOM {
	using T = typename RecType::value_type;

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
	SOM(const Graph &graph, const Metric &metric = Metric(), double start_learn_rate = 0.8,
		double finish_learn_rate = 0.0, size_t iterations = 20, Distribution distribution = Distribution(-1, 1));

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
	 * @param neighborhood_range_decay
	 * @param random_seed
	 */
	SOM(const Graph &graph, const Metric &metric, double start_learn_rate, double finish_learn_rate, size_t iterations,
		Distribution distribution, double neighborhood_start_size, double neighborhood_range_decay,
		long long random_seed);

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
	void train(const std::vector<std::vector<T>> &samples);

	/**
	 * @brief
	 *
	 * @param samples
	 */
	void estimate(const std::vector<std::vector<T>> &samples, const size_t sampleSize);

	/**
	 * @brief
	 *
	 * @param sample
	 * @return
	 */
	std::vector<double> operator()(const RecType &samples) const { return encode(samples); }

	/**
	 * @brief
	 *
	 * @param sample
	 * @return
	 */
	std::vector<double> encode(const RecType &sample) const;

	/**
	 * @brief Best matching unit
	 *
	 * @param sample
	 * @return size_t
	 */
	size_t BMU(const RecType &sample) const;

	/**
	 * @brief
	 *
	 * @return true
	 * @return false
	 */
	bool isValid() const { return valid; }

	/**
	 * @brief
	 *
	 * @param samples
	 *
	 * @return double
	 */
	double std_deviation(const std::vector<std::vector<T>> &samples) const;

	/**
	 * @brief
	 *
	 * @return
	 */
	size_t getNodesNumber() const { return graph.getNodesNumber(); }

	/**
	 * @brief
	 *
	 * @return
	 */
	const std::vector<std::vector<T>> &get_weights() const { return weights; }

	/**
	 * @brief
	 *
	 * @return
	 */
	Graph &get_graph() { return graph; }

	/**
	 * @brief
	 *
	 * @return
	 */
	const Metric &get_metric() const { return metric; }

	/**
	 * @brief
	 *
	 * @param weights
	 */
	void updateWeights(const std::vector<std::vector<T>> &new_weights);

	bool is_weights_changed() { return weights_changed_; }

  protected:
	size_t input_dimensions = 0; // dimensions of inputs vector

	Metric metric;
	Graph graph;
	Distribution distribution = Distribution(-1, 1);

	double start_learn_rate = 0.8;
	double finish_learn_rate = 0.0;
	size_t iterations = 20;

	double neighborhood_start_size;
	double neighborhood_range_decay;
	long long random_seed;

	bool weights_changed_ = false;

	/**
	 * @brief
	 *
	 * @param samples
	 */
	void subsampled_train(const std::vector<std::vector<T>> &samples, int sampleSize);

  private:
	bool valid;
	std::vector<std::vector<T>> weights; // coordinates in the input_dimensions space
};

} // namespace metric
#include "SOM.cpp"
#endif
