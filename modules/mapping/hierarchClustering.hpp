/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_MAPPING_HIERARCHCLUSTERING_HPP
#define _METRIC_MAPPING_HIERARCHCLUSTERING_HPP
#include <vector>

namespace metric {
/**
 * @class Cluster
 * 
 * @brief
 */
template <typename T>
class Cluster {
public:

    explicit Cluster() = default;

    /**
     * @brief Construct a new Cluster object
     *
     * @param d 
     */
    explicit Cluster(const std::vector<T>& d)
        : data(d)
    {
        calculateCentroid();
    }

    explicit Cluster(std::vector<T>&& d)
        : data(d)
    {
        calculateCentroid();
    }

    /**
     * @brief
     *
     */
	template <typename VecType, typename SampleType = std::vector<VecType>>
    void calculateCentroid()
    {
        centroid = SampleType (data[0].size());
        VecType sum;
        for (size_t i = 0; i < data[0].size(); i++) {
            sum = 0;
            for (size_t j = 0; j < data.size(); j++) {
                sum += data[j][i];
            }
            centroid[i] = sum / data.size();
        }
    }

    /**
     * @brief
     *
     */
    void calculateCentroid()
    {
        centroid = data[0];
    }

    /**
     * @brief 
     * 
     */
    std::vector<T> data;

    /**
     * @brief 
     * 
     */
    T centroid;
};

/**
 * @class HierarchicalClustering
 * 
 * @brief 
 * 
 */
template <typename T, typename Distance>
class HierarchicalClustering {
public:
    /**
     * @brief Construct a new Hierarchical Clustering object
     * 
     * @param data 
     * @param k 
     */
    HierarchicalClustering(const std::vector<T>& data, const int& k)
    {
        sourceData = data;
        clustersNum = k;
    }

    /**
     * @brief 
     * 
     */
    void initialize();

    /**
     * @brief 
     * 
     */
    void hierarchical_clustering();

    /**
     * @brief 
     * 
     */
    std::vector<Cluster<T>> clusters;

    /**
     * @brief 
     * 
     */
    std::vector<T> sourceData;

    /**
     * @brief 
     * 
     */
    int clustersNum;

private:
    std::vector<std::vector<double>> calculateDistances();
}; 

}  // namespace clustering

#include "hierarchClustering.cpp"
#endif