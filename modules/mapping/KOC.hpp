/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_MAPPING_KOC_HPP
#define _METRIC_MAPPING_KOC_HPP

#include "SOM.hpp"

#include <array>
#include <vector>

#include <chrono>
#include <random>
#include <algorithm>
#include <cmath>
#include <numeric>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

namespace metric {

/**
 * @class KOC
 *
 *@brief
 *
 */
template <
    typename RecType,
    class Graph = metric::Grid6,
    class Metric = metric::Euclidean<typename RecType::value_type>,
    class Distribution = std::uniform_real_distribution<typename RecType::value_type>
>
class KOC {
    typedef typename RecType::value_type T;

public:

    /**
     * @brief Construct a new KOC object
     *
     * @param graph - pre created graph (with metric::Graph interface) over which SOM is being constructed. Default is metric::Grid6 (hexagones grid).
     * @param metric - metric or distance being used for SOM training. Default is metric::Euclidean.
     * @param anomaly_sigma
     * @param s_learn_rate - start learning rate for SOM training. Shows how error can influence on the sample on the first iterations of the training.
     * @param f_learn_rate - finish learning rate for SOM training. Shows how error can influence on the sample on the last iterations of the training.
     * @param iterations - maximum number of iterations for SOM training.
     * @param distribution - distribution used for creating initial weights (positions) of the SOM nodes.
     */
	KOC(
		const std::vector<RecType>& samples,
		const Graph& graph,
		const Metric& metric,
		int num_clusters,
		double anomaly_sigma = 1.0,
		int min_cluster_size = 1,
		double start_learn_rate = 0.8,
		double finish_learn_rate = 0.0,
		size_t iterations = 20,
		Distribution distribution = Distribution(-1, 1)
	);

    /**
     * @brief Construct a new KOC object
     *
     * @param graph - pre created graph (with metric::Graph interface) over which SOM is being constructed. Default is metric::Grid6 (hexagones grid).
     * @param metric - metric or distance being used for SOM training. Default is metric::Euclidean.
     * @param samples
     * @param num_clusters
     * @param anomaly_sigma
     * @param min_cluster_size
     * @param s_learn_rate - start learning rate for SOM training. Shows how error can influence on the sample on the first iterations of the training.
     * @param f_learn_rate - finish learning rate for SOM training. Shows how error can influence on the sample on the last iterations of the training.
     * @param iterations - maximum number of iterations for SOM training.
     * @param distribution - distribution used for creating initial weights (positions) of the SOM nodes.
     * @param neighborhood_start_size - number of neighbours of the checking node which weights (positions) will be corrected while SOM training.
     * @param neighborhood_range_decay - shows how distance from the checking node influence to weights (positions) of the checking node neighbours while SOM training.
     * @param random_seed - the value used to initialize the random number generator, which is used for the initial distribution of weight (positions).
     */
	KOC(
		const std::vector<RecType>& samples,
		const Graph& graph,
		const Metric& metric,
		int num_clusters,
		double anomaly_sigma,
		int min_cluster_size,
		double start_learn_rate,
		double finish_learn_rate,
		size_t iterations,
		Distribution distribution,
		double neighborhood_start_size,
		double neighborhood_range_decay,
		long long random_seed
	);
	
    /**
     * @brief
     * @param nodesNumber - total number of the nodes for graph over which SOM is being constructed. From this number it should be possible to extract the int square root.
     * @param start_learn_rate - start learning rate for SOM training. Shows how error can influence on the sample on the first iterations of the training.
     * @param finish_learn_rate - finish learning rate for SOM training. Shows how error can influence on the sample on the last iterations of the training.
     * @param iterations - maximum number of iterations for SOM training.
     * @param distribution_min - left side (minimum) of the distribution used for creating initial weights (positions) of the SOM nodes.
     * @param distribution_max - right side (maximum) of the distribution used for creating initial weights (positions) of the SOM nodes.
     */
    KOC(
		const std::vector<RecType>& samples,
        size_t nodesNumber,
		int num_clusters,
        double anomaly_sigma = 1.0,
		int min_cluster_size = 1,
        double start_learn_rate = 0.8,
        double finish_learn_rate = 0.0,
        size_t iterations = 20,
        T distribution_min = -1,
        T distribution_max = 1
    );

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
    KOC(
		const std::vector<RecType>& samples,
        size_t nodesWidth,
        size_t nodesHeight,
		int num_clusters,
        double anomaly_sigma = 1.0,
		int min_cluster_size = 1,
        double start_learn_rate = 0.8,
        double finish_learn_rate = 0.0,
        size_t iterations = 20,
        T distribution_min = -1,
        T distribution_max = 1
    );

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
    KOC(
		const std::vector<RecType>& samples,
        size_t nodesNumber,
		int num_clusters,
		double anomaly_sigma,
		int min_cluster_size,
        double start_learn_rate,
        double finish_learn_rate,
        size_t iterations,
        T distribution_min,
        T distribution_max,
        double neighborhood_start_size,
        double neighborhood_range_decay,
        long long random_seed
    );

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
    KOC(
		const std::vector<RecType>& samples,
        size_t nodesWidth,
        size_t nodesHeight,
		int num_clusters,
		double anomaly_sigma,
		int min_cluster_size,
        double start_learn_rate,
        double finish_learn_rate,
        size_t iterations,
        T distribution_min,
        T distribution_max,
        double neighborhood_start_size,
        double neighborhood_range_decay,
        long long random_seed
    );

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
    void train(const std::vector<RecType>& samples, int num_clusters, int min_cluster_size = 1);

    /**
     * @brief
     *
     * @param samples
     *
     * @return std::vector<bool>
     */
    std::vector<bool> check_if_anomaly(const std::vector<RecType>& samples);

    /**
     * @brief
     *
     * @param sample
     *
     * @return bool
     */
    bool check_if_anomaly(const RecType& sample);

    /**
     * @brief
     *
     * @param samples
     *
     * @return std::vector<int>
     */
    std::vector<int> assign_to_clusters(const std::vector<RecType>& samples);

    /**
     * @brief
     *
     * @param samples
     * @param count
     *
     * @return std::tuple<std::vector<size_t>, std::vector<typename RecType::value_type>> - two vectors, the first with indexes of the top outlier samples (sorted from the farest),
     * and the second with distances to std deviation edge from nodes coordinates (multiplied to anomaly_sigma, according to formula: distance = <distance to the best matchin node> - <std_deviation_of_the_node><anomaly_sigma>)
     */
    std::tuple<std::vector<size_t>, std::vector<typename RecType::value_type>> top_outliers(const std::vector<RecType>& samples, int count = 10);

private:
    int min_cluster_size = 1;
    double anomaly_sigma = 1;
    size_t iterations;
    long long random_seed;

    std::vector<int> clusters;
    std::vector<int> centroids;
    std::vector<int> clusters_counts;

    std::vector<T> nodes_std_deviations;

    Metric metric;
    SOM<RecType, Graph, metric::Euclidean<double>, Distribution> som;

    /**
     * @brief
     *
     * @param samples
     * @param sampleSize
     */
    void calculate_std_deviations_for_nodes(const std::vector<RecType>& samples, int sampleSize);

    /**
     * @brief
     *
     * @param num_clusters
     *
     * @return std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>
     */
    std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> clusterize_nodes(int num_clusters, int min_cluster_size = 1);

    /**
     * @brief
     *
     * @param samples
     * @param sampleSize
     * @param num_clusters
     */
    void estimate(const std::vector<RecType>& samples, const size_t sampleSize, int num_clusters, int min_cluster_size = 1);
};

} // namespace metric

#include "KOC.cpp"

#endif
