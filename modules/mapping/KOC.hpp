/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_MAPPING_KOC_HPP
#define _METRIC_MAPPING_KOC_HPP


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
		template <
			typename recType, 
			class Graph = metric::Grid6, 
			class Metric = metric::Euclidian<typename recType::value_type>, 
			class Distribution = std::uniform_real_distribution<typename recType::value_type>
		>
		class KOC : public SOM<recType, Graph, Metric, Distribution> {

			typedef typename recType::value_type T;

		public:
			/**
				 * @brief Construct a new KOC object
				 * 
				 * @param graph 
				 * @param metric 
				 * @param start_learn_rate 
				 * @param finish_learn_rate 
				 * @param iterations 
				 * @param distribution 
				 */
			KOC(Graph graph, Metric metric, double anomaly_sigma = 1.0, double start_learn_rate = 0.8, double finish_learn_rate = 0.0, size_t iterations = 20, Distribution distribution = Distribution(-1, 1))
				: som_(graph, metric, start_learn_rate, finish_learn_rate, iterations, distribution), 
				anomaly_sigma_(anomaly_sigma), 
				iterations_(iterations),
				random_seed_(std::chrono::system_clock::now().time_since_epoch().count()) {};

			/**
				 * @brief Construct a new KOC object
				 * 
				 * @param graph 
				 * @param metric 
				 * @param start_learn_rate 
				 * @param finish_learn_rate 
				 * @param iterations 
				 * @param distribution 
				 * @param neighborhood_start_size 
				 * @param neigbour_range_decay 
				 * @param random_seed 
				 */
			KOC(Graph graph, Metric metric, double anomaly_sigma, double start_learn_rate, double finish_learn_rate, size_t iterations, 
				Distribution distribution, double neighborhood_start_size, double neigbour_range_decay, long long random_seed)
				: som_(graph, metric, start_learn_rate, finish_learn_rate, iterations, distribution, neighborhood_start_size, neigbour_range_decay, random_seed), 
				anomaly_sigma_(anomaly_sigma), 
				iterations_(iterations), 
				random_seed_(random_seed) {};

			/**
				 * @brief Destroy the KOC object
				 * 
				 */
			~KOC() = default;

			/**
				 * @brief 
				 * 
				 * @param samples 
				 * @param num_clusters 
				 */

			void train(const std::vector<recType>& samples, int num_clusters);

			/**
				 * @brief 
				 * 
				 * @param sample 
				 * @param sigma 
				 *  
				 * @return std::vector<int>
				 */
			std::vector<int> encode(const std::vector<recType>& samples, double sigma = 1.0);


			/**
				 * @brief 
				 * 
				 * @param samples 
				 * @param sigma 
				 * 
				 * @return std::vector<bool> 
				 */

			std::vector<bool> check_if_anomaly(const std::vector<recType>& samples, double sigma = 1.0);


			/**
				 * @brief 
				 * 
				 * @param sample
				 * @param sigma 
				 * 
				 * @return bool
				 */

			bool check_if_anomaly(const recType& sample, double sigma = 1.0);


			/**
				 * @brief 
				 * 
				 * @param samples 

				 * @param sigma 
				 * 
				 * @return std::vector<int>
				 */
			std::vector<int> result(const std::vector<recType>& samples, double sigma = 1.0);

			/**
				 * @brief 
				 * 
				 * @param samples 
				 * @param sigma 
				 * @param count 
				 * 
				 * @return std::tuple<std::vector<size_t>, std::vector<typename recType::value_type>, std::vector<int>>
				 */
			std::tuple<std::vector<size_t>, std::vector<typename recType::value_type>, std::vector<int>> top_outlier(const std::vector<recType>& samples, double sigma = 1.0, int count = 10);

		private:

	
			std::vector<int> clusters;	
			std::vector<recType> centroids;	
			std::vector<int> clusters_counts;	
	
			std::vector<T> nodes_std_deviations;	



			/**
				 * @brief 
				 * 
				 * @param samples 
				 * @param sampleSize 
				 */
			void calculate_std_deviations_for_nodes(const std::vector<recType>& samples, int sampleSize);

			/**
				 * @brief 
				 * 
				 * @param num_clusters 
				 * 
				 * @return std::tuple<std::vector<int>, std::vector<recType>, std::vector<int>>
				 */

			std::tuple<std::vector<int>, std::vector<recType>, std::vector<int>> clusterize_nodes(int num_clusters);


			/**
				 * @brief 
				 * 
				 * @param samples 
				 * @param sampleSize 
				 * @param num_clusters 
				 */
			void estimate(const std::vector<recType>& samples, const size_t sampleSize, int num_clusters);

			/**
				 * @brief 
				 * 
				 * @param sample 
				 * 
				 * @return size_t
				 */
			size_t BMU(const recType& sample) const override;

			/**
				 * @brief 
				 * 
				 * @param sample 
				 * 
				 * @return std::vector<double>
				 */
			std::vector<double> encode(const recType& sample) override;
			
			/**
				 * @brief 
				 * 
				 * @param v 
				 * 
				 * @return std::vector<size_t>
				 */

			template <typename T1>
			std::vector<size_t> sort_indexes(const std::vector<T1> &v);

		};

	}  // namespace KOC_details
	
	//
	
	template <
		typename recType, 
		class Graph = metric::Grid6, 
		class Metric = metric::Euclidian<typename recType::value_type>, 
		class Distribution = std::uniform_real_distribution<typename recType::value_type>
	>
	struct KOC_factory {
		
		typedef typename recType::value_type T;

		/**
			* @brief
			*
			* @param nodesNumber 
			* @param start_learn_rate 
			* @param finish_learn_rate 
			* @param iterations 
			* @param distribution_min 
			* @param distribution_max 
		 */
		KOC_factory(size_t nodesNumber, double anomaly_sigma, double start_learn_rate = 0.8, double finish_learn_rate = 0.0, size_t iterations = 20, T distribution_min = -1, T distribution_max = 1);

		/**
			* @brief
			*
			* @param nodesWidth 
			* @param nodesHeight 
			* @param start_learn_rate 
			* @param finish_learn_rate 
			* @param iterations 
			* @param distribution_min 
			* @param distribution_max 
		 */
		KOC_factory(size_t nodesWidth = 5, size_t nodesHeight = 4, double anomaly_sigma = 1.0, double start_learn_rate = 0.8, double finish_learn_rate = 0.0, size_t iterations = 20, T distribution_min = -1, T distribution_max = 1);

		/**
			* @brief
			*
			* @param nodesNumber 
			* @param start_learn_rate 
			* @param finish_learn_rate 
			* @param iterations 
			* @param distribution_min 
			* @param distribution_max 
			* @param neighborhood_start_size 
			* @param neigbour_range_decay 
			* @param random_seed 
		 */
		KOC_factory(size_t nodesNumber, double anomaly_sigma, 
			double start_learn_rate, double finish_learn_rate, size_t iterations, T distribution_min, T distribution_max, 
			double neighborhood_start_size, double neigbour_range_decay, long long random_seed);

		/**
			* @brief
			*
			* @param nodesWidth 
			* @param nodesHeight 
			* @param start_learn_rate 
			* @param finish_learn_rate 
			* @param iterations 
			* @param distribution_min 
			* @param distribution_max 
			* @param neighborhood_start_size 
			* @param neigbour_range_decay 
			* @param random_seed 
		 */
		KOC_factory(size_t nodesWidth, size_t nodesHeight, double anomaly_sigma, 
			double start_learn_rate, double finish_learn_rate, size_t iterations, T distribution_min, T distribution_max, 
			double neighborhood_start_size, double neigbour_range_decay, long long random_seed);

		/**
			* @brief 
			* 
			* @param samples 
			* @param num_clusters 
			* 
			* @return KOC_details::KOC<recType, Graph, Metric, Distribution>
		 */

		KOC_details::KOC<recType, Graph, Metric, Distribution> operator()(const std::vector<recType>& samples, int num_clusters);


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
		double anomaly_sigma_;
	};

} // namespace metric

#include "KOC.cpp"

#endif
