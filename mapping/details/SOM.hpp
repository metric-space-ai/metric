/*
for specific setting check:
Appropriate Learning Rate and Neighborhood Function of Self-organizing Map (SOM) ...
International Journal of Modeling and Optimization, Vol. 6, No. 1, February 2016
W. Natita, W. Wiboonsak, and S. Dusadee
*/


#pragma once


#include <assert.h>

#include <iostream>
#include <array>
#include <vector>

#include <chrono>
#include <random>
#include <algorithm>
#include <cmath>
#include <numeric>

#include "metric.tpp"
#include "graph.hpp"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif


namespace metric
{

namespace mapping
{
	template <typename recType, class Metric = SOM_details::default_euclidian<recType>, 
			 					class Graph = SOM_details::Grid8>
	class SOM
	{
		typedef typename recType::value_type T;

	  public:
		explicit SOM(size_t nodesNumber, Metric metric = Metric());
		SOM(size_t nodesWidth, size_t nodesHeight, Metric metric = Metric());
		~SOM();

		bool isValid();

		void train(const std::vector<std::vector<T>> &samples,
		           size_t iterations = 10000, double s_learn_rate = 1.2, double f_learn_rate = 0.4);

		std::vector<double> reduce(const recType &sample);


		//recType weightsAt(size_t i, size_t j) const;

		size_t size();

		/* Best matching unit */
		size_t BMU(const recType &sample) const;

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



template <class recType, class Metric, class Graph>
SOM<recType, Metric, Graph>::SOM(size_t nodesNumber, Metric metric)
    : D(0), metric(metric), graph(nodesNumber)
{
    	valid = graph.isValid();
}

template <class recType, class Metric, class Graph>
SOM<recType, Metric, Graph>::SOM(size_t nodesWidth, size_t nodesHeight, Metric metric)
		: D(0), metric(metric), graph(nodesWidth, nodesHeight)
{
	valid = graph.isValid();
}

template <class recType, class Metric, class Graph>
SOM<recType, Metric, Graph>::~SOM() = default;

template<class recType, class Metric, class Graph>
bool SOM<recType, Metric, Graph>::isValid()
{
	return valid;
}

template <class recType, class Metric, class Graph>
void SOM<recType, Metric, Graph>::train(const std::vector<std::vector<T>> &samples,
                                        size_t iterations, double s_learn_rate, double f_learn_rate)
{
	const size_t nodesNumber = graph.getNodesNumber();

    // initialize weight matrix at first training call
    if (D == 0) {
        std::cout << "initial training" << std::endl;


		/* Set sample dimension */
        D = samples[0].size();

		/* Initialize weights */
        weights = std::vector<std::vector<T>>(nodesNumber, std::vector<T>(D));


		/* Create uniform distribution */
		const auto seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine random_generator(seed);
        std::uniform_real_distribution<T> distribution(-1, 1);


		/* Fill weights by uniform distributed values */
		for (auto& weight: weights) {
			for (auto& w: weight) {
                w = distribution(random_generator);
            }
        }

    } else{
        std::cout << "further training" << std::endl;
    }


    assert(D == samples[0].size());


    if (s_learn_rate < f_learn_rate) {
        f_learn_rate = 0;
    }

    double diffLR = s_learn_rate - f_learn_rate;

    //bool less_samples = false;

    //if (samples.size() < iterations) {
     //   less_samples = true;
    //}

	
    /* Random updating */
    std::vector<size_t> randomized_samples(samples.size());
    std::iota(randomized_samples.begin(), randomized_samples.end(), 0);
    std::shuffle(randomized_samples.begin(), randomized_samples.end(), std::mt19937{std::random_device{}()});


	size_t idx=0;
	size_t idx_r=0;

    while (idx < iterations) {

        if (idx_r >= randomized_samples.size() - 1) {
            std::shuffle(randomized_samples.begin(), randomized_samples.end(), std::mt19937{std::random_device{}()});
            idx_r=0;

        } else {
            idx_r++;
        }

        

        //diffLR *= 1.0 / double(idx); // Linear 
        diffLR *= 1.0 - idx / double(iterations); // Inverse of Time Learnrate 
        //diffLR *=  std:exp(idx / double(iterations); // Power Series
        
        double curr_learn_rate = f_learn_rate + diffLR; 

        double neighborhoodSize = (1.0 - idx / double(iterations+1)) * std::sqrt(double(nodesNumber));

        size_t samples_idx = randomized_samples[idx_r];

		/* Get closest node index */
        size_t bmuIndex = BMU(samples[samples_idx]);

        const size_t nSI = std::max(size_t(round(neighborhoodSize)), size_t(0));

        /*size_t min_j = std::max(size_t(0), bmu_idx[1] - nSI);
        size_t min_i = std::max(size_t(0), bmu_idx[0] - nSI);
        size_t max_j = std::min(W - 1, bmu_idx[1] + nSI);
        size_t max_i = std::min(H - 1, bmu_idx[0] + nSI);
		*/

		auto neighbours = graph.getNeighbours(bmuIndex, nSI);


        //update weights of the BMU and its neighborhoods.

		for (size_t deep = 0; deep < neighbours.size(); ++deep) {
			for (size_t i = 0; i < neighbours[deep].size(); ++i) {

				const size_t neighbourIndex = neighbours[deep][i];

                
                //T factor = 1; // uniform
                //T factor = T(calcGaussian2D(bmu_idx[1], bmu_idx[0], T(neighborhoodSize/ 2.0)+0.00000001, j, i)); // Gaussian

                T factor = 1; // uniform
                if (nSI != 0) {
					const T sigma = T(neighborhoodSize/ 2.0);
					factor = std::exp(T(deep * deep) / T(-2 * sigma * sigma));
					//factor = std::exp((T((j - bmu_idx[1]) * (j - bmu_idx[1]) + (i - bmu_idx[0]) * (i - bmu_idx[0]))) / T(- 2 * sigma * sigma));
                }
                //T factor =      std::exp((bmu_idx[1] - i) * (bmu_idx[0] - i) * (bmu_idx[1] - i) * (bmu_idx[0] - i) / (-2.0 * neighborhoodSize * neighborhoodSize)); // ExpDecay

                // if no more neighbours are affected, the factor returns to 1!
                for (size_t k = 0; k < D; ++k) {
                    const T error = samples[samples_idx][k] - weights[neighbourIndex][k];
                    weights[neighbourIndex][k] += T(error * curr_learn_rate * factor);
                }

            }
        }

        //std::cout << std::endl;
        ++idx;
    }
}

template <class recType, class Metric, class Graph>
std::vector<double> 
SOM<recType, Metric, Graph>::reduce(const recType &sample)
{
    std::vector<double> dim_reduced(graph.getNodesNumber());

	for (size_t i = 0; i < dim_reduced.size(); ++i) {
            dim_reduced[i] = metric(sample, weights[i]);
    }

    return dim_reduced;
}


template <class recType, class Metric, class Graph>
size_t
SOM<recType, Metric, Graph>::size()
{
    return graph.getNodesNumber();
}

/** Best matching unit **/
template <class recType, class Metric, class Graph>
size_t SOM<recType, Metric, Graph>::BMU(const recType &sample) const
{
    assert(sample.size() == D); // input sample has not same size than SOM;


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


} // namespace mapping
} // namespace metric
