/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_MAPPING_KOC_HPP
#define _METRIC_MAPPING_KOC_HPP

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

#include "SOM.hpp"
//#include "../distance.hpp"
//#include "../utils/graph.hpp"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

namespace metric {

	namespace KOC_details {

		/**
			 * @class KOC
			 * 
			 *@brief 
			 * 
			 */
		template <typename recType, class Graph = metric::Grid6, class Metric = metric::Euclidian<typename recType::value_type>, class Distribution = std::uniform_real_distribution<typename recType::value_type>>
		class KOC : public SOM<recType, Graph, Metric, Distribution> {
			typedef typename recType::value_type T;

		public:
			/**
				 * @brief Construct a new KOC object
				 * 
				 * @param nodesNumber 
				 * @param metric 
				 */
			KOC(Graph graph, Metric metric, double start_learn_rate = 0.8, double finish_learn_rate = 0.0, size_t iterations = 20, Distribution distribution = Distribution(-1, 1))
				: SOM<recType, Graph, Metric, Distribution>(graph, metric, start_learn_rate, finish_learn_rate, iterations, distribution) {};

			/**
				 * @brief Construct a new KOC object
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
			KOC(Graph graph, Metric metric, double start_learn_rate, double finish_learn_rate, size_t iterations, 
				Distribution distribution, double neighborhood_start_size, double neigbour_range_decay, long long random_seed)
				: SOM<recType, Graph, Metric, Distribution>(graph, metric, start_learn_rate, finish_learn_rate, iterations,  
					distribution, neighborhood_start_size, neigbour_range_decay, random_seed) {};

			/**
				 * @brief Destroy the KOC object
				 * 
				 */
			~KOC() = default;

			/**
				 * @brief 
				 * 
				 * @param samples 
				 */
			void train(const std::vector<std::vector<T>>& samples) override;

			/**
				 * @brief 
				 * 
				 * @param sample 
				 * @return
				 */
			std::vector<int> encode(const std::vector<std::vector<T>>& samples, double anomaly_threshold = 0.0);

			/**
				 * @brief 
				 * 
				 * @param samples 
				 * 
				 * @return std::vector<bool> 
				 */
			std::vector<bool> check_if_anomaly(const std::vector<recType>& samples, double anomaly_threshold = 0.0);

			/**
				 * @brief 
				 * 
				 * @param sample
				 * 
				 * @return bool
				 */
			bool check_if_anomaly(const recType& sample, double anomaly_threshold = 0.0);

			/**
				 * @brief 
				 * 
				 * @return std::vector<int>
				 */
			std::tuple<std::vector<int>, std::vector<std::vector<recType::value_type>>, std::vector<int>> result();

		private:
	
			//double anomaly_threshold_ = 0.0;
			std::vector<int> clusters;	
			std::vector<std::vector<T>> centroids;	
			std::vector<int> clusters_counts;	

			T reduced_mean_entropy;	
			T reduced_min_entropy;	
			T reduced_max_entropy;	
	
			T reduced_mean_closest_distance;	
			T reduced_min_closest_distance;	
			T reduced_max_closest_distance;	

			/**
				 * @brief 
				 * 
				 * @param samples 
				 */
			void parse_distances(const std::vector<std::vector<T>>& samples, int sampleSize);

			/**
				 * @brief 
				 * 
				 * @param samples 
				 */
			std::tuple<std::vector<int>, std::vector<std::vector<recType::value_type>>, std::vector<int>> clusterize_nodes();

			/**
				 * @brief 
				 * 
				 * @param samples 
				 */
			void estimate(const std::vector<std::vector<T>>& samples, const size_t sampleSize) override;

			/**
				 * @brief 
				 * 
				 * @param sample 
				 * @return
				 */
			size_t BMU(const recType& sample) const override;

			/**
				 * @brief 
				 * 
				 * @param sample 
				 * @return
				 */
			std::vector<double> encode(const recType& sample) override;

		};

	}  // namespace KOC_details
	
	//
	
	template <typename recType, class Graph = metric::Grid6, class Metric = metric::Euclidian<typename recType::value_type>, class Distribution = std::uniform_real_distribution<typename recType::value_type>>
	struct KOC_factory {
		
		typedef typename recType::value_type T;

		/**
		 * @brief
		 *
		 */
		KOC_factory(size_t nodesNumber, double start_learn_rate = 0.8, double finish_learn_rate = 0.0, size_t iterations = 20, T distribution_min = -1, T distribution_max = 1);

		/**
		 * @brief
		 *
		 */
		KOC_factory(size_t nodesWidth = 5, size_t nodesHeight = 4, double start_learn_rate = 0.8, double finish_learn_rate = 0.0, size_t iterations = 20, T distribution_min = -1, T distribution_max = 1);

		/**
		 * @brief
		 *
		 */
		KOC_factory(size_t nodesNumber, 
			double start_learn_rate, double finish_learn_rate, size_t iterations, T distribution_min, T distribution_max, 
			double neighborhood_start_size, double neigbour_range_decay, long long random_seed);

		/**
		 * @brief
		 *
		 */
		KOC_factory(size_t nodesWidth, size_t nodesHeight, 
			double start_learn_rate, double finish_learn_rate, size_t iterations, T distribution_min, T distribution_max, 
			double neighborhood_start_size, double neigbour_range_decay, long long random_seed);

		/**
		 * @brief 
		 */
		KOC_details::KOC<recType, Graph, Metric, Distribution> operator()(const std::vector<std::vector<T>>& samples);

	private:

		Metric metric_;
		Graph graph_;
		Distribution distribution_;

		double start_learn_rate_;
		double finish_learn_rate_; 
		size_t iterations_;

		double neighborhood_start_size_;
		double neigbour_range_decay_;
		long long random_seed_;
	};

} // namespace metric

#include "KOC.cpp"

#endif
