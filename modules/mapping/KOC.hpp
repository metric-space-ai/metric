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
		class KOC {
			typedef typename recType::value_type T;

		public:
			/**
				 * @brief Construct a new KOC object
				 * 
				 * @param graph - pre created graph (with metric::Graph interface) over which SOM is being constructed. Default is metric::Grid6 (hexagones grid).
				 * @param metric - metric or distance being used for SOM training. Default is metric::Euclidian.
				 * @param anomaly_sigma
				 * @param s_learn_rate - start learning rate for SOM training. Shows how error can influence on the sample on the first iterations of the training.
				 * @param f_learn_rate - finish learning rate for SOM training. Shows how error can influence on the sample on the last iterations of the training.
				 * @param iterations - maximum number of iterations for SOM training.
				 * @param distribution - distribution used for creating initial weights (positions) of the SOM nodes. 
				 */
			KOC(Graph graph, Metric metric, double anomaly_sigma = 1.0, double start_learn_rate = 0.8, double finish_learn_rate = 0.0, size_t iterations = 20, Distribution distribution = Distribution(-1, 1))
				: som_(graph, metric, start_learn_rate, finish_learn_rate, iterations, distribution),
				anomaly_sigma_(anomaly_sigma),
				iterations_(iterations),
				random_seed_(std::chrono::system_clock::now().time_since_epoch().count()) {};

			/**
				 * @brief Construct a new KOC object
				 * 
				 * @param graph - pre created graph (with metric::Graph interface) over which SOM is being constructed. Default is metric::Grid6 (hexagones grid).
				 * @param metric - metric or distance being used for SOM training. Default is metric::Euclidian.
				 * @param anomaly_sigma
				 * @param s_learn_rate - start learning rate for SOM training. Shows how error can influence on the sample on the first iterations of the training.
				 * @param f_learn_rate - finish learning rate for SOM training. Shows how error can influence on the sample on the last iterations of the training.
				 * @param iterations - maximum number of iterations for SOM training.
				 * @param distribution - distribution used for creating initial weights (positions) of the SOM nodes. 
				 * @param neighborhood_start_size - number of neighbours of the checking node which weights (positions) will be corrected while SOM training. 
				 * @param neigbour_range_decay - shows how distance from the checking node influence to weights (positions) of the checking node neighbours while SOM training. 
				 * @param random_seed - the value used to initialize the random number generator, which is used for the initial distribution of weight (positions).
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
			void train(const std::vector<recType>& samples, int num_clusters, int min_cluster_size = 1);

			/**
				 * @brief 
				 * 
				 * @param samples 
				 * 
				 * @return std::vector<bool> 
				 */
			std::vector<bool> check_if_anomaly(const std::vector<recType>& samples);

			/**
				 * @brief 
				 * 
				 * @param sample
				 * 
				 * @return bool
				 */
			bool check_if_anomaly(const recType& sample);

			/**
				 * @brief 
				 * 
				 * @param samples
				 *
				 * @return std::vector<int>
				 */
			std::vector<int> assign_to_clusters(const std::vector<recType>& samples);

			/**
				 * @brief
				 *
				 * @param samples
				 * @param count
				 *
				 * @return std::tuple<std::vector<size_t>, std::vector<typename recType::value_type>> - two vectors, the first with indexes of the top outlier samples (sorted from the farest),
				 * and the second with distances to std deviation edge from nodes coordinates (multiplied to anomaly_sigma, according to formula: distance = <distance to the best matchin node> - <std_deviation_of_the_node><anomaly_sigma>)
				 */
			std::tuple<std::vector<size_t>, std::vector<typename recType::value_type>> top_outliers(const std::vector<recType>& samples, int count = 10);

		private:
			int min_cluster_size_ = 1;
			double anomaly_sigma_ = 1;
			size_t iterations_;
			long long random_seed_;

			std::vector<int> clusters;	
			std::vector<recType> centroids;
			std::vector<int> clusters_counts;	

			std::vector<T> nodes_std_deviations;

			SOM<recType, Graph, Metric, Distribution> som_;

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
			std::tuple<std::vector<int>, std::vector<recType>, std::vector<int>> clusterize_nodes(int num_clusters, int min_cluster_size = 1);

			/**
				 * @brief 
				 * 
				 * @param samples 
				 * @param sampleSize
				 * @param num_clusters
				 */
			void estimate(const std::vector<recType>& samples, const size_t sampleSize, int num_clusters, int min_cluster_size = 1);

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
		 * @param nodesNumber - total number of the nodes for graph over which SOM is being constructed. From this number it should be possible to extract the int square root.
		 * @param start_learn_rate - start learning rate for SOM training. Shows how error can influence on the sample on the first iterations of the training.
		 * @param finish_learn_rate - finish learning rate for SOM training. Shows how error can influence on the sample on the last iterations of the training.
		 * @param iterations - maximum number of iterations for SOM training.
		 * @param distribution_min - left side (minimum) of the distribution used for creating initial weights (positions) of the SOM nodes. 
		 * @param distribution_max - right side (maximum) of the distribution used for creating initial weights (positions) of the SOM nodes. 
		 */
		KOC_factory(size_t nodesNumber, double anomaly_sigma, double start_learn_rate = 0.8, double finish_learn_rate = 0.0, size_t iterations = 20, T distribution_min = -1, T distribution_max = 1);

		/**
		 * @brief
		 *
		 * @param nodesWidth - number of the nodes in width for graph over which SOM is being constructed.
		 * @param nodesHeight - number of the nodes in height for graph over which SOM is being constructed.
		 * @param start_learn_rate - start learning rate for SOM training. Shows how error can influence on the sample on the first iterations of the training.
		 * @param finish_learn_rate - finish learning rate for SOM training. Shows how error can influence on the sample on the last iterations of the training.
		 * @param iterations - maximum number of iterations for SOM training.
		 * @param distribution_min - left side (minimum) of the distribution used for creating initial weights (positions) of the SOM nodes. 
		 * @param distribution_max - right side (maximum) of the distribution used for creating initial weights (positions) of the SOM nodes. 
		 */
		KOC_factory(size_t nodesWidth = 5, size_t nodesHeight = 4, double anomaly_sigma = 1.0, double start_learn_rate = 0.8, double finish_learn_rate = 0.0, size_t iterations = 20, T distribution_min = -1, T distribution_max = 1);

		/**
		 * @brief
		 *
		 * @param nodesNumber - total number of the nodes for graph over which SOM is being constructed. From this number it should be possible to extract the int square root.
		 * @param start_learn_rate - start learning rate for SOM training. Shows how error can influence on the sample on the first iterations of the training.
		 * @param finish_learn_rate - finish learning rate for SOM training. Shows how error can influence on the sample on the last iterations of the training.
		 * @param iterations - maximum number of iterations for SOM training.
		 * @param distribution_min - left side (minimum) of the distribution used for creating initial weights (positions) of the SOM nodes. 
		 * @param distribution_max - right side (maximum) of the distribution used for creating initial weights (positions) of the SOM nodes. 
		 * @param neighborhood_start_size - number of neighbours of the checking node which weights (positions) will be corrected while SOM training. 
		 * @param neigbour_range_decay - shows how distance from the checking node influence to weights (positions) of the checking node neighbours while SOM training. 
		 * @param random_seed - the value used to initialize the random number generator, which is used for the initial distribution of weight (positions).
		 */
		KOC_factory(size_t nodesNumber, double anomaly_sigma,
			double start_learn_rate, double finish_learn_rate, size_t iterations, T distribution_min, T distribution_max, 
			double neighborhood_start_size, double neigbour_range_decay, long long random_seed);

		/**
		 * @brief
		 *
		 * @param nodesWidth - number of the nodes in width for graph over which SOM is being constructed.
		 * @param nodesHeight - number of the nodes in height for graph over which SOM is being constructed.
		 * @param s_learn_rate - start learning rate for SOM training. Shows how error can influence on the sample on the first iterations of the training.
		 * @param f_learn_rate - finish learning rate for SOM training. Shows how error can influence on the sample on the last iterations of the training.
		 * @param iterations - maximum number of iterations for SOM training.
		 * @param distribution_min - left side (minimum) of the distribution used for creating initial weights (positions) of the SOM nodes. 
		 * @param distribution_max - right side (maximum) of the distribution used for creating initial weights (positions) of the SOM nodes. 
		 * @param neighborhood_start_size - number of neighbours of the checking node which weights (positions) will be corrected while SOM training. 
		 * @param neigbour_range_decay - shows how distance from the checking node influence to weights (positions) of the checking node neighbours while SOM training. 
		 * @param random_seed - the value used to initialize the random number generator, which is used for the initial distribution of weight (positions).
		 */
		KOC_factory(size_t nodesWidth, size_t nodesHeight, double anomaly_sigma,
			double start_learn_rate, double finish_learn_rate, size_t iterations, T distribution_min, T distribution_max, 
			double neighborhood_start_size, double neigbour_range_decay, long long random_seed);

		/**
		 * @brief 
		 * 
		 * @param samples
		 * @param num_clusters - number of expected clusters
		 *
		 * @return KOC_details::KOC<recType, Graph, Metric, Distribution>
		 */
		KOC_details::KOC<recType, Graph, Metric, Distribution> operator()(const std::vector<recType>& samples, int num_clusters, int min_cluster_size = 1);

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
