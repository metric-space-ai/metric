/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#ifndef _METRIC_MAPPING_SOM_CPP
#define _METRIC_MAPPING_SOM_CPP
#include "SOM.hpp"
namespace metric {

template <class recType, class Metric, class Graph, class Distribution>
SOM<recType, Metric, Graph, Distribution>::SOM(size_t nodesNumber, Metric metric)
    : D(0)
    , metric(metric)
    , graph(nodesNumber)
    , distribution(-1, 1)
{
    valid = graph.isValid();

	start_learn_rate = 1.2;
	finish_learn_rate = 0.4;
	iterations = 10000;
	
	random_seed = std::chrono::system_clock::now().time_since_epoch().count();
	
	neighborhood_start_size = std::sqrt(double(nodesNumber));
	neigbour_range_decay = 2.0;
}

template <class recType, class Metric, class Graph, class Distribution>
SOM<recType, Metric, Graph, Distribution>::SOM(size_t nodesWidth, size_t nodesHeight, Metric metric)
    : D(0)
    , metric(metric)
    , graph(nodesWidth, nodesHeight)
    , distribution(-1, 1)
{
    valid = graph.isValid();

	start_learn_rate = 1.2;
	finish_learn_rate = 0.4;
	iterations = 10000;

	random_seed = std::chrono::system_clock::now().time_since_epoch().count();
	
    const size_t nodesNumber = getNodesNumber();
	neighborhood_start_size = std::sqrt(double(nodesNumber));
	neigbour_range_decay = 2.0;
}

template <class recType, class Metric, class Graph, class Distribution>
SOM<recType, Metric, Graph, Distribution>::SOM(Graph graph, Metric metric, double start_learn_rate, double finish_learn_rate, size_t iterations, Distribution distribution)
    : D(0)
    , metric(metric)
    , graph(graph)
    , distribution(distribution)
{
    valid = graph.isValid();

	start_learn_rate = 1.2;
	finish_learn_rate = 0.4;
	iterations = 10000;
	
	random_seed = std::chrono::system_clock::now().time_since_epoch().count();

    const size_t nodesNumber = getNodesNumber();
	neighborhood_start_size = std::sqrt(double(nodesNumber));
	neigbour_range_decay = 2.0;
}

template <class recType, class Metric, class Graph, class Distribution>
SOM<recType, Metric, Graph, Distribution>::SOM(Graph graph, Metric metric, double start_learn_rate,  double finish_learn_rate, size_t iterations, 
	Distribution distribution, double neighborhoodSize, double neigbour_range_decay, long long random_seed)
    : D(0)
    , metric(metric)
    , graph(graph)
    , distribution(distribution)
	, start_learn_rate(start_learn_rate)
	, finish_learn_rate(finish_learn_rate)
	, iterations(iterations)
	, neighborhoodSize(neighborhoodSize)
	, neigbour_range_decay(neigbour_range_decay)
	, random_seed(random_seed)
{
    valid = graph.isValid();
}

//template <class recType, class Metric, class Graph>
//SOM<recType, Metric, Graph>::~SOM() = default;

template <class recType, class Metric, class Graph, class Distribution>
void SOM<recType, Metric, Graph, Distribution>::train(
    const std::vector<std::vector<T>>& samples)
{
    // initialize weight matrix at first training call
    if (D == 0) {

        /* Set sample dimension */
        D = samples[0].size();

        /* Set sample dimension */
        weights = std::vector<std::vector<T>>(getNodesNumber(), std::vector<T>(D));

        /* Create uniform distribution */
        std::default_random_engine random_generator(random_seed);
		
        /* Fill weights by uniform distributed values */
        for (auto& weight : weights) {
            for (auto& w : weight) {
                w = distribution(random_generator);
            }
        }

    }

    assert(D == samples[0].size());

    if (start_learn_rate < finish_learn_rate) {
        finish_learn_rate = 0;
    }

    double diffLR = start_learn_rate - finish_learn_rate;
		
    /* Random updating */
    std::vector<size_t> randomized_samples(samples.size());
    std::iota(randomized_samples.begin(), randomized_samples.end(), 0);
    std::shuffle(randomized_samples.begin(), randomized_samples.end(), std::mt19937 { std::random_device {}() });

    size_t idx = 0;
    size_t idx_r = 0;

    while (idx < iterations) {

        if (idx_r >= randomized_samples.size() - 1) {
            std::shuffle(
                randomized_samples.begin(), randomized_samples.end(), std::mt19937 { std::random_device {}() });
            idx_r = 0;

        } else {
            idx_r++;
        }

        // diffLR *= 1.0 / double(idx); // Linear
        diffLR *= 1.0 - idx / double(iterations);  // Inverse of Time Learnrate
        // diffLR *=  std:exp(idx / double(iterations); // Power Series

        double curr_learn_rate = finish_learn_rate + diffLR;
		
		double neighborhoodSize = (1.0 - idx / double(iterations + 1)) * neighborhood_start_size;

        size_t samples_idx = randomized_samples[idx_r];

        /* Get closest node index */
        size_t bmuIndex = BMU(samples[samples_idx]);

        const size_t nSI = std::max(size_t(round(neighborhoodSize)), size_t(0));

        auto neighbours = graph.getNeighbours(bmuIndex, nSI);

        // update weights of the BMU and its neighborhoods.

        for (size_t deep = 0; deep < neighbours.size(); ++deep) {
            for (size_t i = 0; i < neighbours[deep].size(); ++i) {

                const size_t neighbourIndex = neighbours[deep][i];

                T factor = 1;  // uniform
                if (nSI != 0) {
					
                    const T sigma = T(neighborhoodSize / neigbour_range_decay);
                    factor = std::exp(T(deep * deep) / T(-2 * sigma * sigma));
                }

                // if no more neighbours are affected, the factor returns to 1!
                for (size_t k = 0; k < D; ++k) {
                    const T error = samples[samples_idx][k] - weights[neighbourIndex][k];
                    weights[neighbourIndex][k] += T(error * curr_learn_rate * factor);
                }
            }
        }

        ++idx;
    }
}

template <class recType, class Metric, class Graph, class Distribution>
std::vector<double> SOM<recType, Metric, Graph, Distribution>::encode(const recType& sample)
{
    std::vector<double> dim_reduced(getNodesNumber());

    for (size_t i = 0; i < dim_reduced.size(); ++i) {
        dim_reduced[i] = metric(sample, weights[i]);
    }

    return dim_reduced;
}

/** Best matching unit **/
template <class recType, class Metric, class Graph, class Distribution>
size_t SOM<recType, Metric, Graph, Distribution>::BMU(const recType& sample) const
{
    assert(sample.size() == D);  // input sample has not same getNodesNumber than SOM;

    double minDist = std::numeric_limits<T>::max();
    size_t index = 0;

    for (size_t i = 0; i < weights.size(); ++i) {
        T dist = metric(sample, weights[i]);

        if (dist < minDist) {
            minDist = dist;
            index = i;
        }
    }

    return index;
}

}  // namespace metric

#endif