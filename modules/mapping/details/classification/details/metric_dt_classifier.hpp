#ifndef METRIC_DT_CLASSIFIER_HPP
#define METRIC_DT_CLASSIFIER_HPP
/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Max Filippov
*/
#include <iostream>
#include <vector>
#include <functional>
#include <memory>

#include <math.h> // log2
#include <queue>

// TODO: clear include code out of warnings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
//#include "include/clustering.hpp"
#pragma GCC diagnostic pop


namespace clustering
{


template <typename T>
std::tuple<std::vector<int>,std::vector<int>,std::vector<int>>
kmedoids_(const std::vector<std::vector<T>> & D, //data,
    // T Dsum,
    int k,
    // std::string distance_measure = distance_functions::default_measure()
    int iters = 1000
    );


} // namespace clustering





namespace metric
{

namespace classification
{



template <class Record>
class MetricDT
{

  private:

    struct Node // of the tree
    {
        // std::shared_ptr<Node> parent = nullptr;
        std::vector<std::shared_ptr<Node>> children = {};
        // std::vector<int> medoid_indices = {};
        std::vector<Record> medoid_records = {};
        int field_index = -1;
        int predicted_class = -1; // used in leafs
        std::vector<int> prediction_distribution = {};
    };

    struct NodeDataUnit // queue is used instead of recursion; we need subset only in queue when building tree, not in resulting tree
    {
        std::shared_ptr<Node> node; //
        std::shared_ptr<std::vector<int>> subset;
        double entropy = 0;
        int debug_id;
    };


  public:

    MetricDT(double entropy_threshold_=0, double gain_threshold_=0);


    //template <typename ConType, typename DimSet> // code for DimensionSet
    template <typename ConType, typename VariantType>
    void train(
        ConType & payments,
        // std::vector<std::function<NumType(Record)>> & features,  // (old) code for accessors
        //DimSet & dimensions, // code for DimensionSet
        std::vector<VariantType>,
        std::function<int(const Record&)> & response);


    //template <typename ConType, typename DimSet>// code for DimensionSet
    template <typename ConType, typename VariantType>
    void predict(
            ConType & input_data,
            // std::vector<std::function<NumType(Record)> > & features, // (old) code for accessors
            //DimSet & dimensions, // code for DimensionSet
            std::vector<VariantType> dimensions,
            std::vector<int> & predictions);

  private:

    template <typename NumType>
    std::tuple< std::vector<std::vector<NumType>>, std::vector<int> > distance_matrix_of_subset(
            const std::vector<int> & subset,
            const std::vector<std::vector<NumType>> & feature_dist
            );


    template <typename ConType, typename NumType>
    std::tuple<std::vector<std::shared_ptr<std::vector<int>>>, std::vector<double>, std::vector<int>, int, double> split_subset(
            const std::vector<int> & subset,
            const std::vector<std::vector<std::vector<NumType>>> & distances,
            const ConType & data,
            const std::function<int(Record)> & response
            );


    template <typename ConType, typename NumType>
    std::tuple< std::vector<std::vector<std::vector<NumType>>>, std::vector<int> > read_data( // TODO return without copying!!
        ConType & payments,
        std::vector<std::function<NumType(Record)>> & features,
            std::function<int(Record)> & response);


    template <typename ConType>
    inline void add_distribution_to_node( // mutates *new_node!
            const ConType & payments,
            const std::function<int(Record)> & response,
            const std::shared_ptr<std::vector<int>> & new_subset,
            const std::shared_ptr<Node> & new_node // subject to change
            );


    // private properties:

    std::vector<int> unique_labels = {};
    std::shared_ptr<Node> root = nullptr;
    double entropy_threshold = 0;
    double gain_threshold = 0;

};





} // namespace classification
} // namespace metric

#include "metric_dt_classifier.cpp"

#endif // METRIC_DT_CLASSIFIER_HPP
