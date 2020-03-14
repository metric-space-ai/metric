/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2018 Max Filippov
*/
#include "../DT.hpp"
#include "dimension.hpp"
#include "../../kmedoids.hpp"
#include <cassert>
#include <exception>

namespace metric {

template <class Record>
DT<Record>::DT(double entropy_threshold_, double gain_threshold_)
{
    entropy_threshold = entropy_threshold_;
    gain_threshold = gain_threshold_;
}

template <class Record>
template <typename ConType, typename VariantType>
void DT<Record>::train(
    const ConType& payments, std::vector<VariantType> dimensions, std::function<int(const Record&)>& response)
{
    typedef double NumType;  // TODO replace hardcode

    if (dimensions.size() == 0)
        return;

    // first by feature, then record against record matrix
    std::vector<std::vector<std::vector<NumType>>> distances = {};
    for (size_t f = 0; f < dimensions.size(); f++)  // field (dimension) loop
    {
        std::vector<std::vector<NumType>> matrix(payments.size(), std::vector<NumType>(payments.size()));
        for (size_t i = 0; i < payments.size(); i++) {
            for (size_t j = i; j < payments.size(); j++) {
                auto d_dist_vis
                    = [&payments, i, j](auto& d) { return (NumType)d.get_distance(payments[i], payments[j]); };
                matrix[i][j] = matrix[j][i] = std::visit(d_dist_vis, dimensions[f]);
            }
            bool found = false;  // also count unique labels
            int r = response(payments[i]);
            if (r < 0) {
                throw std::runtime_error("Error in input dataset: negative labels are not allowed. Learning cancelled");
                return;
            }
            for (int l : unique_labels) {
                if (l == r) {
                    found = true;
                    break;
                }
            }
            if (!found)
                unique_labels.push_back(r);
        }
        distances.push_back(matrix);
    }
    // (new) code for dimensions - end

    // here distances table is fullfilled for each feature

    // now we can start growing the tree from the root
    std::queue<NodeDataUnit> subset_queue;  // this is instead of recursion
    std::vector<int> root_subset;  // first node subset
    for (size_t i = 0; i < payments.size(); i++)
        root_subset.push_back((int)i);  // indices of subset: initially all records in dataset are selected

    std::shared_ptr<Node> current_node = std::make_shared<Node>();
    root = current_node;
    NodeDataUnit root_unit;
    root_unit.node = current_node;
    root_unit.subset = std::make_shared<std::vector<int>>(root_subset);  // TODO make ctor from data size, if needed
    root_unit.entropy = std::numeric_limits<double>::max();  // root element will be processed anyway
    root_unit.debug_id = 0;  // for debug purpose, TODO remove
    subset_queue.push(root_unit);

    int unit_debug_id = 0;

    while (!subset_queue.empty()) {
        NodeDataUnit unit = subset_queue.front();
        subset_queue.pop();

        auto [new_subsets, new_entropies, new_medoids, field, entropy_weighted_sum]
            = split_subset(*unit.subset, distances, payments, response);
        // gain-based condition check
        double gain = unit.entropy - entropy_weighted_sum;

        if (gain <= gain_threshold) {
            // add leaf without processing subsets
            add_distribution_to_node(payments, response, unit.subset, unit.node);  // changes *unit.node
            continue;
        }

        for (size_t i = 0; i < new_subsets.size();
             i++)  // loop through subsets obtained from clustering function via process_node
        {  // we assume sizes of all vectors in tuple are eqeual (to the number of classes in train dataset)
            unit.node->medoid_records.push_back(payments[new_medoids[i]]);  // add medoid raw record
            unit.node->field_index = field;  // field the division is made by
            if (new_entropies[i] <= entropy_threshold || new_subsets[i]->size() < unique_labels.size()) {
                // add leaf node
                auto new_node = std::make_shared<Node>();
                if (!(new_entropies[i] > 0)) {
                    if ((*(new_subsets[i])).size() > 0) {
                        // add non-stohastic leaf node
                        new_node->predicted_class
                            = response(payments[(*(new_subsets[i]))[0]]);  // call label accessor for the first element
                        // assume subset is not empty and all labels are equal (because of zero entropy)
                    } else {
                        // empty subset ocurred, add stohastic equally-distributed node
                        for (size_t l = 0; l < unique_labels.size(); l++)
                            new_node->prediction_distribution.push_back(1);
                    }

                } else {
                    add_distribution_to_node(payments, response, new_subsets[i], new_node);  // changes *new_node
                }
                unit.node->children.push_back(new_node);  // set pointer to the newly created child in the current node
            } else {
                // enqueue regular node (add subset to queue and node to tree)
                NodeDataUnit new_unit;
                new_unit.node = std::make_shared<Node>();  // create new empty node within queue unit
                new_unit.subset = new_subsets[i];  // pointer to vector of subset indices
                new_unit.entropy = new_entropies[i];  // in order to compute gain on the next step
                unit.node->children.push_back(
                    new_unit.node);  // set pointer to the newly created child in the current node
                new_unit.debug_id = ++unit_debug_id;  // for debuf porpose, TODO disable
                subset_queue.push(new_unit);  // enqueue new data unit;
            }
        }
    }
}

template <class Record>
template <typename ConType, typename VariantType>
void DT<Record>::predict(const ConType& input_data, std::vector<VariantType> dimensions, std::vector<int>& predictions)
{
    typedef double NumType;  // TODO replace hardcode

    if (root == nullptr || input_data.size() == 0)
        return;

    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());

    for (size_t i = 0; i < input_data.size(); i++)  // input data item loop  (new) code for dimensions
    {
        std::shared_ptr<Node> current_node = root;
        while (current_node->children.size() > 0)  //predicted_class == -1) // go through tree until leaf is reached
        {
            // check if children found but predicted class is defined
            assert(current_node->predicted_class == -1);

            double min_distance = std::numeric_limits<double>::max();
            int nearese_medoid_index = -1;
            for (size_t m = 0; m < current_node->medoid_records.size(); m++)  // medoid loop
            {
                Record r = current_node->medoid_records[m];
                auto d_dist_vis = [&input_data, i, r](auto& d) { return (NumType)d.get_distance(input_data[i], r); };
                NumType distance = std::visit(d_dist_vis, dimensions[current_node->field_index]);

                if (distance < min_distance) {
                    min_distance = distance;
                    nearese_medoid_index = m;
                }
            }
            // here we have nearest medoid index, so we can chose the child, assuming order of childs is same as order of medoids

            current_node = current_node->children[nearese_medoid_index];
        }
        if (current_node->predicted_class != -1)
            predictions.push_back(current_node->predicted_class);
        else {
            // montecarlo prediction generation based on leaf subset label distribution
            int r_max = 0;
            std::vector<int> roulette = {};
            for (int el : current_node->prediction_distribution) {
                r_max += el;
                roulette.push_back(r_max);
            }
            std::uniform_int_distribution<int> distr(0, r_max - 1);
            int random_val = (int)distr(generator);
            int r_idx = 0;
            while (random_val >= roulette[r_idx])
                r_idx++;
            // here r_idx is randomely chosen index in current_node->prediction_distribution

            predictions.push_back(unique_labels[r_idx]);
        }
    }
}

template <class Record>
template <typename NumType>
std::tuple<std::vector<std::vector<NumType>>, std::vector<int>> DT<Record>::distance_matrix_of_subset(
    const std::vector<int>& subset, const std::vector<std::vector<NumType>>& feature_dist)
{
    // output - distance subtable ready for kmedoids_
    std::vector<std::vector<NumType>> result(subset.size(), std::vector<NumType>(subset.size()));
    std::vector<int> orig_idx(subset.size());
    for (size_t i = 0; i < subset.size(); i++) {
        for (size_t j = i; j < subset.size(); j++) {
            result[i][j] = result[j][i] = feature_dist[subset[i]][subset[j]];
        }
        orig_idx[i] = subset[i];
    }
    return std::tuple(result, orig_idx);
}

template <class Record>
template <typename ConType, typename NumType>
std::tuple<std::vector<std::shared_ptr<std::vector<int>>>, std::vector<double>, std::vector<int>, int, double>
DT<Record>::split_subset(const std::vector<int>& subset,
    const std::vector<std::vector<std::vector<NumType>>>& distances, const ConType& data,
    const std::function<int(Record)>& response)
{
    // input - indices of subset, full distance matrices for all features, raw data, label acceessor
    // output - for each new subset: pointer to vector of element indices, entropy value, medoid index, and single values: index of the field used for classification and entropy weighted sum

    std::vector<std::vector<std::vector<int>>> new_subsets(
        distances.size(), std::vector<std::vector<int>>(unique_labels.size(), std::vector<int>(0)));
    // by field, then by nearest medoid, then global indices of new subset elements

    std::vector<std::vector<double>> split_entropy(distances.size(), std::vector<double>(unique_labels.size(), 0));
    // by field, then by nearest medoid (one value per new subset for each field)
    // we keep entropy values of all fields in order to return it for the winning one

    // std::vector<double> entropy_weighted_sum(distances.size(), 0);
    // one value per field

    std::vector<std::vector<int>> medoid_global_idx(distances.size(), std::vector<int>(unique_labels.size(), -1));
    // by field, then by nearest medoid
    // global indices of medoids for each field

    double min_entropy_weighted_sum = std::numeric_limits<double>::max();  // criterion for the best field selection
    int greatest_information_gain_idx = -1;

    for (size_t f = 0; f < distances.size(); f++)  // field loop
    {
        //  clustering via kmedoids_

        auto [feature_dist_table, feature_dist_orig_idx] = distance_matrix_of_subset(subset, distances[f]);

        auto results = metric::kmedoids_(feature_dist_table, unique_labels.size(), 1000);
        auto medoids = std::get<1>(
            results);  // medoid indices in feature_dist_table; original indices can be obtained via feature_dist_orig_idx

        std::vector<std::vector<int>> label_count(unique_labels.size(), std::vector<int>(medoids.size(), 0));
        // one value per each class for each new subset

        for (auto el : subset)  // record loop
        {
            // find and compare distances from the current record to medoids
            int chosen_medoid = -1;
            NumType min_dist;
            for (size_t medoid_idx = 0; medoid_idx < medoids.size(); medoid_idx++)  // medoid loop
            {  // compare distance to medoids. We assume medoids.size() MUST NOT be greater than unique_labels.size()
                NumType d
                    = distances[f][el][feature_dist_orig_idx[medoids[medoid_idx]]];  // between each el and each medoid

                if (chosen_medoid == -1)  // first time
                {
                    min_dist = d;
                    chosen_medoid = medoid_idx;
                } else {
                    if (d < min_dist) {
                        min_dist = d;
                        chosen_medoid = medoid_idx;
                    }
                }
            }  // end of medoid loop
            // here we have index of the nearest mediod in chosen medoid; this index is related to 'medoids' vector, where indices in feature_dist_table are saved

            // add el to new subset related to the nearest medoid and count labels for each new subset
            new_subsets[f][chosen_medoid].push_back(el);  // add the current element to appropriate new subset
            for (size_t l = 0; l < unique_labels.size(); l++) {
                if (unique_labels[l] == response(data[el])) {
                    label_count
                        [l]
                        [chosen_medoid]++;  // query response and count elements of each class (by labels) in each new subset
                    break;  // values in label_count are unique
                }
            }
        }  // end of record loop
        // here we have subsets for the current field based on distance to medoids listed in 'medoids' vector
        // and counts of elements of each class

        double current_entropy_weighted_sum = 0;
        for (size_t medoid_idx = 0; medoid_idx < medoids.size();
             medoid_idx++)  // medoid loop: collecting outpot data for each new subset
        {  // now we do not know which field will win, so we collect data for each field
            // find entropy of each new subset and weighted sum of them (gain subtrahend)
            double new_entropy = 0;
            for (size_t l = 0; l < unique_labels.size(); l++) {
                double probability = (double)label_count[l][medoid_idx]
                    / new_subsets[f][medoid_idx].size();  // share of each class by labels
                if (probability > 0)
                    new_entropy = new_entropy - probability * std::log2(probability);
                // else do nothing, because lim(share*log2(share))==0
                // TODO add zero entropy "leaf reached" flag if needed

                //Something went wrong: negative entropy
                assert(new_entropy >= 0);
                assert(!isnan(new_entropy));
            }
            split_entropy[f][medoid_idx] = new_entropy;
            current_entropy_weighted_sum
                = current_entropy_weighted_sum + new_entropy * new_subsets[f][medoid_idx].size() / subset.size();

            // also fill global medoid indices (in order to output them for winning field)
            medoid_global_idx[f][medoid_idx] = feature_dist_orig_idx[medoids[medoid_idx]];
        }  // end of medoid loop
        if (current_entropy_weighted_sum < min_entropy_weighted_sum)  // update winning field
        {
            min_entropy_weighted_sum = current_entropy_weighted_sum;
            greatest_information_gain_idx = f;
        }
        // here we also have entropy of each subset and gain subtrahend for all subsets (filled for the current field)

    }  // end of field loop

    // here we have new subsets, entropy of each subset and gain subtrahends - for all fields
    // and index of the winning field

    // make output ptrs
    std::vector<std::shared_ptr<std::vector<int>>> output_subset_ptrs = {};
    for (
        size_t medoid_idx = 0; medoid_idx < unique_labels.size();
        medoid_idx++)  // medoid loop: making ptrs: doing it only for winning field mast be faster than creating ptrs for all field inside record loop
        output_subset_ptrs.push_back(
            std::make_shared<std::vector<int>>(new_subsets[greatest_information_gain_idx][medoid_idx]));

    return std::tuple(output_subset_ptrs,  // new_subsets[best_field_idx],
        split_entropy[greatest_information_gain_idx], medoid_global_idx[greatest_information_gain_idx],
        greatest_information_gain_idx, min_entropy_weighted_sum);
}

template <class Record>
template <typename ConType>
inline void DT<Record>::add_distribution_to_node(  // mutates *new_node!
    const ConType& payments, const std::function<int(Record)>& response,
    const std::shared_ptr<std::vector<int>>& new_subset,
    const std::shared_ptr<Node>& new_node  // subject to change
)
{
    if (new_subset->size() < 1)
        std::cout << "\nSomething went wrong: empty dataset obtained!\n";  // should never happen
    // describe distribution in order to generate random values in predict function
    for (size_t l = 0; l < unique_labels.size(); l++)
        new_node->prediction_distribution.push_back(0);  // init distribution
    for (size_t r = 0; r < new_subset->size(); r++)  // elements in subset
    {  // fullfill distribution vector
        for (size_t l = 0; l < unique_labels.size(); l++)
            if (response(payments[(*new_subset)[r]]) == unique_labels[l])
                new_node->prediction_distribution[l]++;  // simply count occurences
    }
}

}  // namespace metric
