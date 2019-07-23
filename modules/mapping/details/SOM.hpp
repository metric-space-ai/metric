#pragma once
/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_MAPPING_DETAILS_SOM_HPP
#define _METRIC_MAPPING_DETAILS_SOM_HPP

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
#include "../../distance.hpp"
#include "../../../utils/graph/graph.hpp"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif


namespace metric
{

namespace mapping
{
    template <typename recType, class Metric = metric::distance::Euclidian<typename recType::value_type>,
                                                                class Graph = graph::Grid8> // replaced SOM_details with graph by Max F, 2019-05-16
	class SOM
	{
		typedef typename recType::value_type T;

	  public:
		explicit SOM(size_t nodesNumber, Metric metric = Metric());
		SOM(size_t nodesWidth, size_t nodesHeight, Metric metric = Metric());
		~SOM() = default;

		bool isValid();

		void train(const std::vector<std::vector<T>> &samples,
		           size_t iterations = 10000, double s_learn_rate = 1.2, double f_learn_rate = 0.4);

		std::vector<double> reduce(const recType &sample);


		//recType weightsAt(size_t i, size_t j) const;

		size_t size();

		/* Best matching unit */
		size_t BMU(const recType &sample) const;

		std::vector<std::vector<recType::value_type>> nodes();

	  private:
		bool valid;
		size_t D; // dimensions of inputs vector

		Metric metric;
		Graph graph;

		std::vector<std::vector<T>> weights; // nodes of SOM
											 // T calcGaussian(T mean, T stdDev, T x) const;

		// T calcGaussian2D(T meanX, T meanY,
		// 				 T sigmaX, T sigmaY, T x, T y) const;


		//double calcGaussian2D(size_t meanX, size_t meanY, T sigma, size_t x, size_t y) const;

	};


} // namespace mapping
} // namespace metric
#include "SOM.cpp"
#endif
