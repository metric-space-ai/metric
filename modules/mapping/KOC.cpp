/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#ifndef _METRIC_MAPPING_KOC_CPP
#define _METRIC_MAPPING_KOC_CPP
#include "KOC.hpp"
namespace metric {

template <class recType, class Graph, class Metric, class Distribution>
void KOC<recType, Graph, Metric, Distribution>::train(const std::vector<std::vector<T>>& samples)
{
    SOM<recType, Graph, Metric, Distribution>::train(samples);
    parse_distances(samples, samples.size());
    clusters = clusterize_nodes();
}

template <class recType, class Graph, class Metric, class Distribution>
void KOC<recType, Graph, Metric, Distribution>::estimate(
    const std::vector<std::vector<T>>& samples, const size_t sampleSize)
{
    SOM<recType, Graph, Metric, Distribution>::estimate(samples, sampleSize);
    parse_distances(samples, sampleSize);
    clusters = clusterize_nodes();
}

template <class recType, class Graph, class Metric, class Distribution>
std::vector<double> KOC<recType, Graph, Metric, Distribution>::encode(const recType& sample)
{
    return SOM<recType, Graph, Metric, Distribution>::encode(sample);
}

template <class recType, class Graph, class Metric, class Distribution>
std::vector<int> KOC<recType, Graph, Metric, Distribution>::encode(const std::vector<std::vector<T>>& samples)
{
    std::vector<int> result;

    for (size_t i = 0; i < samples.size(); i++) {
        if (check_if_anomaly(samples[i])) {
            result.push_back(0);
        } else {
            //auto reduced = SOM<recType, Graph, Metric, Distribution>::encode(samples[i]);
            auto bmu = SOM<recType, Graph, Metric, Distribution>::BMU(samples[i]);
            result.push_back(clusters[bmu]);
        }
    }

    return result;
}

template <class recType, class Graph, class Metric, class Distribution>
size_t KOC<recType, Graph, Metric, Distribution>::BMU(const recType& sample) const
{
    return SOM<recType, Graph, Metric, Distribution>::BMU(sample);
}

template <class recType, class Graph, class Metric, class Distribution>
std::vector<bool> KOC<recType, Graph, Metric, Distribution>::check_if_anomaly(const std::vector<recType>& samples)
{
    std::vector<bool> result;

    auto entropy_range = reduced_mean_entropy - (reduced_mean_entropy - reduced_min_entropy) * (1 + anomaly_threshold_);
    std::cout << "entropy_range: " << entropy_range << " reduced_mean_entropy: " << reduced_mean_entropy
              << " reduced_min_entropy: " << reduced_min_entropy << " reduced_max_entropy: " << reduced_max_entropy
              << std::endl;

    for (size_t i = 0; i < samples.size(); i++) {
        // if entropy less then min entropy level then it is anomaly
        result.push_back(check_if_anomaly(samples[i]));
    }

    return result;
}

template <class recType, class Graph, class Metric, class Distribution>
bool KOC<recType, Graph, Metric, Distribution>::check_if_anomaly(const recType& sample)
{
    //auto closest_distance_range = reduced_mean_closest_distance - (reduced_mean_closest_distance - reduced_min_closest_distance) * (1 + anomaly_threshold_);
    auto entropy_range = reduced_mean_entropy - (reduced_mean_entropy - reduced_min_entropy) * (1 + anomaly_threshold_);

    auto reduced = SOM<recType, Graph, Metric, Distribution>::encode(sample);
    auto bmu = SOM<recType, Graph, Metric, Distribution>::BMU(sample);
    // if closest distance more then max closest distance level then it is anomaly
    // return reduced[bmu] > closest_distance_range;

    std::sort(reduced.begin(), reduced.end());

    std::vector<std::vector<T>> reduced_reshaped;
    for (size_t j = 0; j < reduced.size(); j++) {
        reduced_reshaped.push_back({ reduced[j] });
    }
    auto e = entropy(reduced_reshaped, 3, 2.0, SOM<recType, Graph, Metric, Distribution>::metric);

    // if entropy less then min entropy level then it is anomaly
    return e < entropy_range;
}

// PRIVATE

template <class recType, class Graph, class Metric, class Distribution>
void KOC<recType, Graph, Metric, Distribution>::parse_distances(
    const std::vector<std::vector<T>>& samples, int sampleSize)
{
    if (sampleSize > samples.size()) {
        sampleSize = samples.size();
    }
    // Random samples
    std::vector<size_t> randomized_samples(samples.size());
    std::iota(randomized_samples.begin(), randomized_samples.end(), 0);

    // shuffle samples after all was processed
    std::shuffle(randomized_samples.begin(), randomized_samples.end(), std::mt19937 { std::random_device {}() });

    std::vector<T> entropies;
    std::vector<T> closest_distances;  // closest distances to the nodes for each sample from train dataset
    T entropies_sum = 0;
    T distances_sum = 0;
    for (size_t i = 0; i < sampleSize; i++) {
        size_t sample_idx = randomized_samples[i];

        auto reduced = SOM<recType, Graph, Metric, Distribution>::encode(samples[sample_idx]);
        auto bmu = SOM<recType, Graph, Metric, Distribution>::BMU(samples[sample_idx]);
        distances_sum += reduced[bmu];
        closest_distances.push_back(reduced[bmu]);

        std::sort(reduced.begin(), reduced.end());

        std::vector<recType> reduced_reshaped;
        for (size_t j = 0; j < reduced.size(); j++) {
            reduced_reshaped.push_back({ reduced[j] });
        }
        auto e = entropy(reduced_reshaped, 3, 2.0, SOM<recType, Graph, Metric, Distribution>::metric);
        entropies_sum += e;
        entropies.push_back(e);
    }

    reduced_mean_entropy = entropies_sum / entropies.size();
    auto result = std::max_element(entropies.begin(), entropies.end());
    reduced_max_entropy = entropies[std::distance(entropies.begin(), result)];
    result = std::min_element(entropies.begin(), entropies.end());
    reduced_min_entropy = entropies[std::distance(entropies.begin(), result)];

    reduced_mean_closest_distance = distances_sum / closest_distances.size();
    result = std::max_element(closest_distances.begin(), closest_distances.end());
    reduced_max_closest_distance = closest_distances[std::distance(closest_distances.begin(), result)];
    result = std::min_element(closest_distances.begin(), closest_distances.end());
    reduced_min_closest_distance = closest_distances[std::distance(closest_distances.begin(), result)];
}

template <class recType, class Graph, class Metric, class Distribution>
std::vector<int> KOC<recType, Graph, Metric, Distribution>::clusterize_nodes()
{
    int min_cluster_size = 0;
    int num_clusters = KOC<recType, Graph, Metric, Distribution>::getNodesNumber();

    auto nodes_data = KOC<recType, Graph, Metric, Distribution>::get_weights();

    while (min_cluster_size <= 1) {
        // clustering on the reduced data

        auto [assignments, exemplars, counts] = metric::kmeans(nodes_data, num_clusters, 1000);

        std::vector<int>::iterator result = std::min_element(counts.begin(), counts.end());
        min_cluster_size = counts[std::distance(counts.begin(), result)];

        int num_ones = 0;
        for (auto i : counts) {
            if (i <= 1) {
                num_ones++;
            }
        }

        int new_num_clusters = num_clusters * min(0.5, 0.1 * num_ones);
        if (new_num_clusters == num_clusters) {
            new_num_clusters--;
        }

        num_clusters = new_num_clusters;

        if (num_clusters <= 1) {
            for (size_t i = 0; i < assignments.size(); i++) {
                // we want 0 label for anomalies, so increment original labels
                assignments[i]++;
            }
            return assignments;
        }
    }

    return std::vector<int>();
}

}  // namespace metric

#endif