/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2021 Panda Team
*/

#ifndef _METRIC_MAPPING_CSAD_HPP
#define _METRIC_MAPPING_CSAD_HPP

#include <vector>
#include <string>


namespace metric {


template <typename T>
class ClusteringSomAnomalyDetector {


public:

    ClusteringSomAnomalyDetector(const std::string & filename);

    ClusteringSomAnomalyDetector(    // create by training using features
            const std::vector<std::vector<T>> & ds,
            const size_t num_clusters = 4, //10;
            const unsigned int som_iterations = 100, //10000,
            const double som_initial_neighbour_size = 5,
            const double som_neigbour_range_decay = 2,
            const int som_w_grid_size = 30,
            const int som_h_grid_size = 30,
            const double som_start_learn_rate = 0.8,
            const double som_final_learn_rate = 0.2,
            const long long som_random_seed = 0,
            //const std::vector<uint32_t> w_sizes = {12, 24, 48, 96, 192, 384},  // TODO save to JSON
            const uint32_t samples = 1000,
            const double confidence_level = 0.99
            );

    void save(const std::string & filename);

    std::vector<T> encode(const std::vector<std::vector<T>> & dataset, T entropy_threshold = 0.8);

    std::vector<std::vector<T>> encode_debug(const std::vector<std::vector<T>> & dataset, T entropy_threshold = 0.8);


private:

    std::vector<T>  // numbers of points that fall into each cluster (by BMUs)
    cluster_entries(
        const std::vector<std::vector<T>> & dataset,
        const size_t pos_idx, // position just after window
        const size_t wnd_size
        // and maybe metric type as template parameter
        );

    //T
    std::tuple<T, T, T>
    anomaly_score(
        const std::vector<std::vector<T>> & dataset,
        const size_t pos_idx, // position just after window
        const size_t wnd_size,
        const size_t cl_idx
        //...
        );

    std::vector<uint32_t> window_sizes = {12, 24, 48, 96, 192, 384};

    std::vector<std::vector<std::vector<std::vector<std::vector<T>>>>> conf_bounds;
    // clusters, subbands, window sizes, bound types (left, med, right), quant values

    std::vector<std::vector<std::vector<T>>> nodes;
    // clusters, nodes, node features

    std::vector<std::vector<std::vector<std::vector<T>>>> cdfs;
    // clusters, subbands, CDF vectors (values, probabilities), CDF vector values

}; // class clustering_som_anomaly_detector






}  // namespace metric


#include "clustering_som_anomaly_detector.cpp"

#endif
