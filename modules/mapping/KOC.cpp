/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include "KOC.hpp"
#include "kmedoids.hpp"
#include "../../modules/space/matrix.hpp"

namespace metric {

template <typename T>
std::vector<size_t> sort_indexes(const std::vector<T> &v)
{
    // initialize original index locations
    std::vector<size_t> idx(v.size());
    std::iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    std::sort(idx.begin(), idx.end(),
        [&v](size_t i1, size_t i2) {return v[i1] > v[i2];});

    return idx;
}

template <class RecType, class Graph, class Metric, class Distribution>
void KOC<RecType, Graph, Metric, Distribution>::train(
    const std::vector<RecType>& samples, int num_clusters, int min_cluster_size)
{
    som.train(samples);
    calculate_std_deviations_for_nodes(samples, samples.size());
    std::tie(clusters, centroids, clusters_counts) = clusterize_nodes(num_clusters, min_cluster_size);
}


//template <class RecType, class Graph, class Metric, class Distribution>
//void KOC<RecType, Graph, Metric, Distribution>::estimate(const std::vector<RecType>& samples, const size_t sampleSize, int num_clusters, int min_cluster_size)
//{
//	som_.estimate(samples, sampleSize);
//	calculate_std_deviations_for_nodes(samples, sampleSize);
//	std::tie(clusters, centroids, clusters_counts) = clusterize_nodes(num_clusters, min_cluster_size);
//}


template <class RecType, class Graph, class Metric, class Distribution>
std::vector<bool> KOC<RecType, Graph, Metric, Distribution>::check_if_anomaly(const std::vector<RecType>& samples)
{
    std::vector<bool> result;

    for (size_t i = 0; i < samples.size(); i++)
    {
        // if entropy less then min entropy level then it is anomaly
        result.push_back(check_if_anomaly(samples[i]));
    }

    return result;
}

template <class RecType, class Graph, class Metric, class Distribution>
bool KOC<RecType, Graph, Metric, Distribution>::check_if_anomaly(const RecType& sample)
{
    auto reduced = som.encode(sample);
    auto bmu = som.BMU(sample);
    // if closest distance more then max closest distance level then it is anomaly
    return reduced[bmu] - node_means[bmu] > nodes_std_deviations[bmu] * anomaly_sigma;
}


template <class RecType, class Graph, class Metric, class Distribution>
std::vector<int> KOC<RecType, Graph, Metric, Distribution>::assign_to_clusters(
    const std::vector<RecType>& samples)
{
    std::vector<int> assignments;
    auto anomalies = check_if_anomaly(samples);
    for (size_t i = 0; i < samples.size(); i++)
    {
        // we want 0 label as anomaly
        if (anomalies[i])
        {
            assignments.push_back(0);
        }
        else
        {
            auto bmu = som.BMU(samples[i]);
            assignments.push_back(clusters[bmu]);
        }
    }

    return assignments;
};


template <class RecType, class Graph, class Metric, class Distribution>
std::tuple<std::vector<size_t>, std::vector<typename RecType::value_type>>
    KOC<RecType, Graph, Metric, Distribution>::top_outliers(const std::vector<RecType>& samples, int count)
{
    auto assignments = assign_to_clusters(samples);

    std::vector<T> distances;
    for (int i = 0; i < samples.size(); i++)
    {
        auto reduced = som.encode(samples[i]);
        auto bmu = som.BMU(samples[i]);
        distances.push_back(reduced[bmu] - nodes_std_deviations[bmu] * anomaly_sigma);
    }

    auto idxs = sort_indexes(distances);
    std::vector<T> sorted_distances;
    //std::vector<int> sorted_assignments;
    if (idxs.size() > count)
    {
        idxs.resize(count);
    }
    for (int i = 0; i < idxs.size(); i++)
    {
        sorted_distances.push_back(distances[idxs[i]]);
        //sorted_assignments.push_back(assignments[idxs[i]]);
    }

    return {idxs, sorted_distances};
};


/// PRIVATE

template <class RecType, class Graph, class Metric, class Distribution>
void KOC<RecType, Graph, Metric, Distribution>::calculate_std_deviations_for_nodes(const std::vector<RecType>& samples, int sampleSize)
{
    if (sampleSize > samples.size())
    {
        sampleSize = samples.size();
    }
    // Random samples
    std::vector<size_t> randomized_samples(samples.size());
    std::iota(randomized_samples.begin(), randomized_samples.end(), 0);

    // shuffle samples after all was processed

    std::default_random_engine random_generator(random_seed);

    std::shuffle(randomized_samples.begin(), randomized_samples.end(), random_generator);

    int num_nodes = som.getNodesNumber();

    std::vector<int> closest_distances(num_nodes, 0);
    std::vector<T> square_distances_sum(num_nodes, 0);
    node_means = std::vector<T>(num_nodes);
    for (size_t i = 0; i < sampleSize; i++)
    {
        size_t sample_idx = randomized_samples[i];

        auto sample = next(samples.begin(), sample_idx);
        auto reduced = som.encode(*sample);
        auto bmu = som.BMU(*sample);

        node_means[bmu] += reduced[bmu];
        closest_distances[bmu]++;
    }
	for (size_t i = 0; i < num_nodes; i++)
	{
		if (closest_distances[i] > 0)
		{
			node_means[i] /= closest_distances[i];
		}
	}
    for (size_t i = 0; i < sampleSize; i++)
    {
        size_t sample_idx = randomized_samples[i];

        auto sample = next(samples.begin(), sample_idx);
        auto reduced = som.encode(*sample);
        auto bmu = som.BMU(*sample);

        square_distances_sum[bmu] += (reduced[bmu] - node_means[bmu]) * (reduced[bmu] - node_means[bmu]);
    }

    nodes_std_deviations = std::vector<T>(num_nodes);
    for (size_t i = 0; i < num_nodes; i++)
    {
        if (closest_distances[i] > 0)
        {
            nodes_std_deviations[i] = sqrt(square_distances_sum[i] / closest_distances[i]);
        }
        else
        {
            nodes_std_deviations[i] = 0;
        }
    }
}

template <class RecType, class Graph, class Metric, class Distribution>
std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> KOC<RecType, Graph, Metric, Distribution>::clusterize_nodes(int num_clusters, int min_cluster_size)
{
    int current_min_cluster_size = -1;

    auto nodes_data = som.get_weights();

    if (min_cluster_size > nodes_data.size())
    {
        min_cluster_size = nodes_data.size();
    }

    while (current_min_cluster_size < min_cluster_size)
    {
        // clustering on the reduced data

        metric::Matrix<RecType, Metric> matrix(nodes_data, metric);
        auto[assignments, seeds, counts] = metric::kmedoids(matrix, num_clusters);

        std::vector<int>::iterator result = std::min_element(counts.begin(), counts.end());
        current_min_cluster_size = counts[std::distance(counts.begin(), result)];

        int num_ones = 0;
        for (auto i : counts)
        {
            if (i <= 1)
            {
                num_ones++;
            }
        }

        int new_num_clusters = num_clusters * std::min(0.5, 0.1 * num_ones);
        if (new_num_clusters == num_clusters)
        {
            new_num_clusters--;
        }
        if (new_num_clusters <= 0)
        {
            new_num_clusters = 1;
        }

        num_clusters = new_num_clusters;

        if (current_min_cluster_size >= min_cluster_size)
        {
            for (size_t i = 0; i < assignments.size(); i++)
            {
                // we want 0 label as anomaly, so increment original labels
                assignments[i]++;
            }

            return { assignments, seeds, counts };
        }
    }

    return { std::vector<int>(), std::vector<int>(), std::vector<int>() };
}


template <class RecType, class Graph, class Metric, class Distribution>
KOC<RecType, Graph, Metric, Distribution>::KOC(
	const std::vector<RecType>& samples, 
    const Graph& graph,
    const Metric& metric,
	int num_clusters, 
    double anomaly_sigma,
	int min_cluster_size, 
    double start_learn_rate,
    double finish_learn_rate,
    size_t iterations,
    Distribution distribution
)
    : 
	som(
		graph, 
		metric::Euclidean<double>(), 
		start_learn_rate, 
		finish_learn_rate, 
		iterations, 
		distribution
	), 
	metric(metric), 
	anomaly_sigma(anomaly_sigma), 
	iterations(iterations), 
	random_seed(std::chrono::system_clock::now().time_since_epoch().count())
{
	train(samples, num_clusters, min_cluster_size);
}

template <class RecType, class Graph, class Metric, class Distribution>
KOC<RecType, Graph, Metric, Distribution>::KOC(
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
)
    : 
	som(
		graph, 
		metric::Euclidean<double>(), 
		start_learn_rate, 
		finish_learn_rate, 
		iterations, 
		distribution, 
		neighborhood_start_size, 
		neighborhood_range_decay, 
		random_seed
	), 
	metric(metric), 
	anomaly_sigma(anomaly_sigma), 
    min_cluster_size(min_cluster_size),
	iterations(iterations), 
	random_seed(random_seed)
{
	train(samples, num_clusters, min_cluster_size);
}

//

template <class RecType, class Graph, class Metric, class Distribution>
KOC<RecType, Graph, Metric, Distribution>::KOC(
	const std::vector<RecType>& samples, 
	size_t nodesNumber, 
	int num_clusters, 
    double anomaly_sigma,
	int min_cluster_size, 
    double start_learn_rate, 
	double finish_learn_rate, 
	size_t iterations, 
	T distribution_min, 
	T distribution_max
) 
	:
    metric(),
    anomaly_sigma(anomaly_sigma),
    min_cluster_size(min_cluster_size),
    iterations(iterations),
    random_seed(std::chrono::system_clock::now().time_since_epoch().count())
{
	auto graph = Graph(nodesNumber);
	auto distribution = Distribution(distribution_min, distribution_max);
	auto neighborhood_start_size = std::sqrt(double(nodesNumber));
	auto neighborhood_range_decay = 2.0;
	
	som = SOM<RecType, Graph, metric::Euclidean<double>, Distribution>(
		graph,
		metric::Euclidean<double>(),
		start_learn_rate,
		finish_learn_rate,
		iterations,
		distribution,
		neighborhood_start_size,
		neighborhood_range_decay,
		random_seed
		);

	train(samples, num_clusters, min_cluster_size);
}

template <class RecType, class Graph, class Metric, class Distribution>
KOC<RecType, Graph, Metric, Distribution>::KOC(
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
	T distribution_max
) 
	:
    metric(),
    anomaly_sigma(anomaly_sigma),
    min_cluster_size(min_cluster_size),
    iterations(iterations),
    random_seed(std::chrono::system_clock::now().time_since_epoch().count())
{
	auto graph = Graph(nodesWidth * nodesHeight);
	auto distribution = Distribution(distribution_min, distribution_max);
	auto neighborhood_start_size = std::sqrt(double(nodesWidth * nodesHeight));
	auto neighborhood_range_decay = 2.0;
	
	som = SOM<RecType, Graph, metric::Euclidean<double>, Distribution>(
		graph,
		metric::Euclidean<double>(),
		start_learn_rate,
		finish_learn_rate,
		iterations,
		distribution,
		neighborhood_start_size,
		neighborhood_range_decay,
		random_seed
		);

	train(samples, num_clusters, min_cluster_size);
}

template <class RecType, class Graph, class Metric, class Distribution>
KOC<RecType, Graph, Metric, Distribution>::KOC(
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
) 
	:
	som(
		Graph(nodesNumber),
		metric::Euclidean<double>(), 
		start_learn_rate, 
		finish_learn_rate, 
		iterations, 
		Distribution(distribution_min, distribution_max),
		neighborhood_start_size, 
		neighborhood_range_decay, 
		random_seed
	), 
    metric(),
    anomaly_sigma(anomaly_sigma),
    min_cluster_size(min_cluster_size),
    iterations(iterations),
    random_seed(random_seed)
{
	train(samples, num_clusters, min_cluster_size);
}

template <class RecType, class Graph, class Metric, class Distribution>
KOC<RecType, Graph, Metric, Distribution>::KOC(
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
) 
	:
	som(
		Graph(nodesWidth, nodesHeight),
		metric::Euclidean<double>(), 
		start_learn_rate, 
		finish_learn_rate, 
		iterations, 
		Distribution(distribution_min, distribution_max),
		neighborhood_start_size, 
		neighborhood_range_decay, 
		random_seed
	), 
    metric(),
    anomaly_sigma(anomaly_sigma),
    min_cluster_size(min_cluster_size),
    iterations(iterations),
    random_seed(random_seed)
{
	train(samples, num_clusters, min_cluster_size);
}

}  // namespace metric
