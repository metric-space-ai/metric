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

template <class recType, class Graph, class Metric, class Distribution>
SOM<recType, Graph, Metric, Distribution>::SOM(size_t nodesNumber, Metric metric)
    : input_dimensions(0)
    , metric(metric)
    , graph(nodesNumber)
    , distribution(-1, 1)
{
    valid = graph.isValid();

	start_learn_rate = 0.8;
	finish_learn_rate = 0.0;
	iterations = 20;
	
	random_seed = std::chrono::system_clock::now().time_since_epoch().count();
	
	neighborhood_start_size = std::sqrt(double(nodesNumber));
	neigbour_range_decay = 2.0;
}

template <class recType, class Graph, class Metric, class Distribution>
SOM<recType, Graph, Metric, Distribution>::SOM(size_t nodesWidth, size_t nodesHeight, Metric metric)
    : input_dimensions(0)
    , metric(metric)
    , graph(nodesWidth, nodesHeight)
    , distribution(-1, 1)
{
    valid = graph.isValid();

	start_learn_rate = 0.8;
	finish_learn_rate = 0.0;
	iterations = 20;

	random_seed = std::chrono::system_clock::now().time_since_epoch().count();
	
    const size_t nodesNumber = getNodesNumber();
	neighborhood_start_size = std::sqrt(double(nodesNumber));
	neigbour_range_decay = 2.0;
}

template <class recType, class Graph, class Metric, class Distribution>
SOM<recType, Graph, Metric, Distribution>::SOM(Graph graph, Metric metric, double start_learn_rate, double finish_learn_rate, size_t iterations, Distribution distribution)
    : input_dimensions(0)
    , metric(metric)
    , graph(graph)
    , distribution(distribution)
    , start_learn_rate(start_learn_rate)
    , finish_learn_rate(finish_learn_rate)
    , iterations(iterations)
{
    valid = graph.isValid();
	
	random_seed = std::chrono::system_clock::now().time_since_epoch().count();

    const size_t nodesNumber = getNodesNumber();
	neighborhood_start_size = std::sqrt(double(nodesNumber));
	neigbour_range_decay = 2.0;
}

template <class recType, class Graph, class Metric, class Distribution>
SOM<recType, Graph, Metric, Distribution>::SOM(Graph graph, Metric metric, double start_learn_rate,  double finish_learn_rate, size_t iterations, 
	Distribution distribution, double neighborhood_start_size, double neigbour_range_decay, long long random_seed)
    : input_dimensions(0)
    , metric(metric)
    , graph(graph)
    , distribution(distribution)
	, start_learn_rate(start_learn_rate)
	, finish_learn_rate(finish_learn_rate)
	, iterations(iterations)
	, neighborhood_start_size(neighborhood_start_size)
	, neigbour_range_decay(neigbour_range_decay)
	, random_seed(random_seed)
{
    valid = graph.isValid();
}

//template <class recType, class Metric, class Graph>
//SOM<recType, Metric, Graph>::~SOM() = default;

template <class recType, class Graph, class Metric, class Distribution>
void SOM<recType, Graph, Metric, Distribution>::train(
    const std::vector<std::vector<T>>& samples)
{
	subsampled_train_(samples, samples.size());
	parse_distances(samples);
}


template <class recType, class Graph, class Metric, class Distribution>
void SOM<recType, Graph, Metric, Distribution>::estimate(const std::vector<std::vector<T>>& samples, const size_t sampleSize)
{
	subsampled_train_(samples, sampleSize);
	parse_distances(samples);
}

template <class recType, class Graph, class Metric, class Distribution>
std::vector<double> SOM<recType, Graph, Metric, Distribution>::encode(const recType& sample)
{
    std::vector<double> dim_reduced(getNodesNumber());

    for (size_t i = 0; i < dim_reduced.size(); ++i) {
        dim_reduced[i] = metric(sample, weights[i]);
    }

    return dim_reduced;
}

/** Best matching unit **/
template <class recType, class Graph, class Metric, class Distribution>
size_t SOM<recType, Graph, Metric, Distribution>::BMU(const recType& sample) const
{
    assert(sample.size() == input_dimensions);  // input sample has not same getNodesNumber than SOM;

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


template <class recType, class Graph, class Metric, class Distribution>
double SOM<recType, Graph, Metric, Distribution>::std_deviation(const std::vector<std::vector<T>>& samples)
{
	double total_distances = 0;
	double std_deviation = 0;
	double closest_distance;

	total_distances = 0;
	for (size_t i = 0; i < samples.size(); i++)
	{
		auto dimR = encode(samples[i]);
		auto bmu = BMU(samples[i]);
		// dimR[bmu] - is distance to the closest node, we use it as difference of value and mean of the values
		closest_distance = dimR[bmu] * dimR[bmu];
		total_distances += closest_distance;
	}

	return sqrt(total_distances / samples.size());
}


template <class recType, class Graph, class Metric, class Distribution>
std::vector<bool> SOM<recType, Graph, Metric, Distribution>::check_if_anomaly(const std::vector<std::vector<T>>& samples, double samples_threshold)
{
	std::vector<bool> result;
	
	int threshold_index = closest_distances.size() * (1 - samples_threshold);
	if (threshold_index == closest_distances.size())
	{
		threshold_index--;
	}
	std::cout << threshold_index << " " << closest_distances.size() << " " << closest_distances[threshold_index] << " " << std::endl;

	for (size_t i = 0; i < samples.size(); i++)
	{
		auto dimR = encode(samples[i]);
		auto bmu = BMU(samples[i]);

		result.push_back(dimR[bmu] > closest_distances[threshold_index]);
	}

	return result;
}

// PRIVATE

template <class recType, class Graph, class Metric, class Distribution>
void SOM<recType, Graph, Metric, Distribution>::subsampled_train_(const std::vector<std::vector<T>>& samples, const size_t sampleSize)
{
	
    // initialize weight matrix at first training call
    if (input_dimensions == 0) {

        // set sample dimension 
        input_dimensions = samples[0].size();

        // init weights
        weights = std::vector<std::vector<T>>(getNodesNumber(), std::vector<T>(input_dimensions));

		std::default_random_engine random_generator(random_seed);
		
        // Fill weights by uniform distributed values 
        for (auto& weight : weights) {
            for (auto& w : weight) {
                w = distribution(random_generator);
            }
        }

    }

    assert(input_dimensions == samples[0].size());

    if (start_learn_rate < finish_learn_rate) {
        finish_learn_rate = 0;
    }

    double learn_rate_base = start_learn_rate - finish_learn_rate;
		
    // Random updating 
    std::vector<size_t> randomized_samples(samples.size());
    std::iota(randomized_samples.begin(), randomized_samples.end(), 0);

    size_t idx = 0;
    //size_t idx_r = 0;

    while (idx < iterations) {

		// shuffle samples after all was processed		
        std::shuffle(randomized_samples.begin(), randomized_samples.end(), std::mt19937 { std::random_device {}() });

		for (auto idx_r = 0; idx_r < randomized_samples.size(); idx_r++)
		{
			//if (idx_r >= randomized_samples.size() - 1) {
			//    std::shuffle(
			//        randomized_samples.begin(), randomized_samples.end(), std::mt19937 { std::random_device {}() });
			//    idx_r = 0;

			//} else {
			//    idx_r++;
			//}

			// break if we use subsampling (i.e. train on a part of the whole samples)
			if (idx_r >= sampleSize)
				break;

			// learn_rate_base *= 1.0 / double(idx); // Linear
			// learn_rate_base *= 1.0 - idx / double(iterations);  // Inverse of Time Learnrate
			// learn_rate_base *=  std:exp(idx / double(iterations); // Power Series

			double progress_invert_stage = (1.0 - idx / double(iterations));

			double curr_learn_rate = progress_invert_stage * learn_rate_base + finish_learn_rate;

			double neighborhood_size = progress_invert_stage * neighborhood_start_size;

			size_t samples_idx = randomized_samples[idx_r];

			// Get the closest node index 
			size_t bmu_index = BMU(samples[samples_idx]);

			const size_t neighbours_num = std::max(size_t(round(neighborhood_size)), size_t(0));

			auto neighbours = graph.getNeighbours(bmu_index, neighbours_num);

			// update weights of the BMU and its neighborhoods.

			for (size_t deep = 0; deep < neighbours.size(); ++deep) {
				for (size_t i = 0; i < neighbours[deep].size(); ++i) {

					const size_t neighbour_index = neighbours[deep][i];

					T remoteness_factor = 1;
					// if no more neighbours are affected, the remoteness_factor returns to 1!
					if (neighbours_num != 0) {

						const T sigma = T(neighborhood_size / neigbour_range_decay);
						remoteness_factor = std::exp(T(deep * deep) / T(-2 * sigma * sigma));
					}

					// correct coordinates in the input_dimensions space (in other words: weights) depends from the error
					for (size_t k = 0; k < input_dimensions; ++k) {
						const T error = samples[samples_idx][k] - weights[neighbour_index][k];
						weights[neighbour_index][k] += T(error * curr_learn_rate * remoteness_factor);
					}
				}
			}
		}
		
		++idx;		

    }
}



template <class recType, class Graph, class Metric, class Distribution>
void SOM<recType, Graph, Metric, Distribution>::parse_distances(const std::vector<std::vector<T>>& samples)
{
	for (size_t i = 0; i < samples.size(); i++)
	{
		auto dimR = encode(samples[i]);
		auto bmu = BMU(samples[i]);
		closest_distances.push_back(dimR[bmu]);
	}
	
    std::sort(closest_distances.begin(), closest_distances.end());
}

}  // namespace metric



#endif