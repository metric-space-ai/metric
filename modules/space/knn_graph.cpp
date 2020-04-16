/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team

*/

#ifndef _METRIC_SPACE_KNN_GRAPH_CPP
#define _METRIC_SPACE_KNN_GRAPH_CPP
#include "knn_graph.hpp"
namespace metric {
template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
template <typename Container, typename>
KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::KNNGraph(
    const Container& samples, size_t neighbors_num, size_t max_bruteforce_size, int max_iterations, double update_range)
    : Graph<WeightType, isDense, isSymmetric>(samples.size())
    , _nodes(samples)
    , _neighbors_num(neighbors_num)
    , _max_bruteforce_size(max_bruteforce_size)
    , _max_iterations(max_iterations)
    , _update_range(update_range)
{
    construct(samples);
}

template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::KNNGraph(const KNNGraph& graph)
    : Graph<WeightType, isDense, isSymmetric>()
    , _neighbors_num(graph._neighbors_num)
    , _max_bruteforce_size(graph._max_bruteforce_size)
    , _max_iterations(graph._max_iterations)
    , _update_range(graph._update_range)
{
    _nodes = graph._nodes;
    // copy assignment
    this->m = graph.m;
    this->valid = true;
}


template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
template <typename Container, typename>
void KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::calculate_distance_matrix(const Container& samples)
{
    Distance distance;

    for (int i = 0; i < samples.size(); i++) {
        // take each node
        auto i_point = samples[i];
        std::vector<distance_value_type_t> distances;
        // then calculate distances for all other nodes
        for (int j = 0; j < samples.size(); j++) {
            auto i_other_point = samples[j];
            distances.push_back(distance(i_point, i_other_point));
        }

        _distance_matrix.push_back(distances);
    }
}

template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
template <typename Container, typename>
void KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::make_edge_pairs(const Container& samples)
{
    this->m.resize(samples.size(), samples.size());
    std::vector<int> ids(samples.size());
    std::iota(ids.begin(), ids.end(), 0);

    std::vector<std::pair<size_t, size_t>> edgesPairs;
    double updated_percent = 1.0;
    int iterations = 0;

    // we iterate until we can found new edges (updated edges between nodes)
    while (updated_percent > _update_range) {
        // create or update approximated knn graph
        auto newEdgesPairs = random_pair_division(samples, ids, _max_bruteforce_size);

        // then update edge pair and check how many was updated
        int was_size = edgesPairs.size();
        for (int j = 0; j < newEdgesPairs.size(); j++) {
            bool already_exist = false;

            for (int k = 0; k < edgesPairs.size(); k++) {
                if (edgesPairs[k] == newEdgesPairs[j]
                    || (edgesPairs[k].first == newEdgesPairs[j].second
                        && edgesPairs[k].second == newEdgesPairs[j].first)) {
                    already_exist = true;
                    break;
                }
            }

            if (!already_exist) {
                edgesPairs.push_back(newEdgesPairs[j]);
            }
        }
        if (was_size > 0) {
            updated_percent = (double)(edgesPairs.size() - was_size) / was_size;
        }

        iterations++;

        if (iterations >= _max_iterations) {
            break;
        }
    }

    // finish graph
    this->buildEdges(edgesPairs);
}

template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
template <typename Container, typename>
void KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::construct(const Container& samples)
{
    calculate_distance_matrix(samples);

    make_edge_pairs(samples);

    this->valid = true;
}

template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
template <typename Container, typename>
std::vector<std::pair<size_t, size_t>>
KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::random_pair_division(
    const Container& samples, const std::vector<int>& ids, int max_size)
{
    Distance d;
    std::vector<Sample> A;
    std::vector<Sample> B;
    std::vector<int> A_ids;
    std::vector<int> B_ids;
    std::vector<std::pair<size_t, size_t>> edgesPairs;
    std::vector<std::pair<size_t, size_t>> edgesPairsResult;

    auto n = samples.size();

    if (n > 0) {
        std::random_device rnd;
        std::mt19937 mt(rnd());
        std::uniform_int_distribution<int> dist(0, n - 1);

        if (n <= max_size) {
            // conquer stage
            edgesPairs = brute_force(samples, ids);
        } else {
            // divide stage

            // take random nodes(samples)
            auto a = samples[dist(mt)];
            auto b = samples[dist(mt)];
            Sample x;
            // and divide all nodes to two groups, where each node is close to one of two initial points
            for (int i = 0; i < n; i++) {
                x = samples[i];
                if (d(x, a) < d(x, b)) {
                    A.push_back(x);
                    A_ids.push_back(ids[i]);
                } else {
                    B.push_back(x);
                    B_ids.push_back(ids[i]);
                }
            }

            // and recursively divide both groups again
            edgesPairsResult = random_pair_division(A, A_ids, max_size);
            edgesPairs.insert(edgesPairs.end(), edgesPairsResult.begin(), edgesPairsResult.end());
            edgesPairsResult = random_pair_division(B, B_ids, max_size);
            edgesPairs.insert(edgesPairs.end(), edgesPairsResult.begin(), edgesPairsResult.end());
        }
    }

    return edgesPairs;
}

template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
template <typename Container, typename>
std::vector<std::pair<size_t, size_t>> KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::brute_force(
    const Container& samples, const std::vector<int>& ids)
{
    std::vector<std::pair<size_t, size_t>> edgesPairs;

    int update_count = 0;
    std::vector<std::vector<distance_value_type_t>> distances;
    std::vector<distance_value_type_t> distance_row;
    for (int i = 0; i < ids.size(); i++) {
        distance_row.clear();
        for (int j = 0; j < ids.size(); j++) {
            distance_row.push_back(_distance_matrix[i][j]);
        }
        distances.push_back(distance_row);
    }

    for (int i = 0; i < ids.size(); i++) {
        auto idxs = sort_indexes(distances[i]);

        for (int j = 0; j < idxs.size(); j++) {
            // omit first item because it is pair between the same item
            if (j == 0) {
                continue;
            }

            // and break if we get '_neighbors_num' edges
            if (j >= _neighbors_num) {
                break;
            }

            bool already_exist = false;

            std::vector<int>::const_iterator max_index = std::max_element(ids.begin(), ids.end());
            // here we keep number of edges from each node
            std::vector<int> num_edges_by_node(ids[std::distance(ids.begin(), max_index)] + 1, 0);
            for (int k = 0; k < edgesPairs.size(); k++) {
                num_edges_by_node[edgesPairs[k].first]++;
                num_edges_by_node[edgesPairs[k].second]++;
                if (edgesPairs[k] == std::pair<size_t, size_t>(ids[i], ids[idxs[j]])
                    || edgesPairs[k] == std::pair<size_t, size_t>(ids[idxs[j]], ids[i])) {
                    already_exist = true;
                    break;
                }
                // if we want to keep neighbours strickt not more then _neighbors_num
                if (_not_more_neighbors) {
                    if (num_edges_by_node[edgesPairs[k].first] >= _neighbors_num
                        || num_edges_by_node[edgesPairs[k].second] >= _neighbors_num) {
                        already_exist = true;
                        break;
                    }
                }
            }

            // add current node and closest to result
            if (!already_exist) {
                edgesPairs.emplace_back(ids[i], ids[idxs[j]]);
            }
        }
    }

    return edgesPairs;
}

template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
std::vector<size_t> KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::gnnn_search(
    const Sample& query, int max_closest_num, int iterations, int num_greedy_moves, int num_expansions)
{
    std::vector<size_t> result;

    // variables for choosen nodes during search
    std::vector<distance_value_type_t> choosen_distances;
    std::vector<int> choosen_nodes;

    // temp variables
    std::vector<distance_value_type_t> distances;
    distance_value_type_t distance;

    Distance distancer;

    // num_expansions should be less then k(neighbors_num) of the graph
    if (num_expansions > _neighbors_num) {
        num_expansions = _neighbors_num;
    }

    // if params missed
    if (num_expansions < 0) {
        num_expansions = _neighbors_num;
    }
    // if still negative
    if (num_expansions < 0) {
        num_expansions = 1;
    }
    if (num_greedy_moves < 0) {
        // if param is missed we choose 20% of all nodes as number of mooves
        num_greedy_moves = round(_nodes.size() * 0.2);
    }

    //

    if (_nodes.size() > 0) {
        for (int i = 0; i < iterations; i++) {

            std::random_device rnd;
            std::mt19937 mt(rnd());
            std::uniform_int_distribution<int> dist(0, _nodes.size() - 1);

            // get initial random node from the graph
            int checking_node = dist(mt);
            int prev_node = -1;
            int new_node;

            // walk from initial node on distance 'num_greedy_moves' steps
            for (int j = 0; j < num_greedy_moves; j++) {
                distances.clear();
                // 0 index is for node itself, 1 - is first circle of neighbours
                auto neighbours = this->getNeighbours(checking_node, 1)[1];

                // get first num_expansions neighbours for the checking node and calculate distances to the query
                for (int p = 0; p < num_expansions; p++) {
                    if (p < neighbours.size()) {
                        distance = distancer(_nodes[neighbours[p]], query);
                        distances.push_back(distance);

                        if (std::find(choosen_nodes.begin(), choosen_nodes.end(), neighbours[p])
                            == choosen_nodes.end()) {
                            choosen_distances.push_back(distance);
                            choosen_nodes.push_back(neighbours[p]);
                        }
                    }
                }

                auto min_index = std::min_element(distances.begin(), distances.end());
                new_node = neighbours[std::distance(distances.begin(), min_index)];
                // if we back to the visited node then we fall in loop and search is complete
                if (new_node == prev_node) {
                    break;
                }
                prev_node = checking_node;
                checking_node = new_node;
            }
        }

        // sort distances and return corresopnding nodes from choosen
        auto idxs = sort_indexes(choosen_distances);
        for (int i = 0; i < max_closest_num; i++) {
            if (i < idxs.size()) {
                result.push_back(choosen_nodes[idxs[i]]);
            }
        }
    }

    return result;
}

template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
template <typename T1>
std::vector<size_t> KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::sort_indexes(const std::vector<T1>& v)
{
    // initialize original index locations
    std::vector<size_t> idx(v.size());
    std::iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    std::sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) { return v[i1] < v[i2]; });

    return idx;
}
template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
std::size_t KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::insert(const Sample& p)
{
    _nodes.push_back(p);
    construct(_nodes);
    return _nodes.size() - 1;
}
template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
template <typename Container, typename>
std::vector<std::size_t> KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::insert(const Container& p)
{
    auto sz = _nodes.size();
    _nodes.insert(_nodes.end(), std::begin(p), std::end(p));
    construct(_nodes);
    std::vector<std::size_t> res(p.size());
    std::iota(res.begin(), res.end(), sz);
    return res;
}
template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
std::pair<std::size_t, bool> KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::insert_if(
    const Sample& p, typename Distance::distance_type threshold)
{
    auto nn = gnnn_search(p, 1);
    Distance metric;
    if (metric(_nodes[nn[0]], p) < threshold)
        return std::pair { 0, false };
    auto id = insert(p);
    return std::pair { id, true };
}

template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
template <typename Container, typename>
std::vector<std::pair<std::size_t, bool>> KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::insert_if(
    const Container& items, typename Distance::distance_type threshold)
{
    Distance metric;
    std::vector<std::pair<std::size_t, bool>> v;
    v.reserve(items.size());
    std::size_t id = _nodes.size();
    for (auto& i : items) {
        auto nn = gnnn_search(i, 1);
        if (metric(_nodes[nn[0]], i) < threshold) {
            v.emplace_back(0, false);
        } else {
            v.emplace_back(id, true);
            //            _nodes.push_back(i);
            id++;
        }
    }
    for (std::size_t i = 0; i < items.size(); i++) {
        if (v[i].second == true) {
            _nodes.push_back(items[i]);
        }
    }
    construct(_nodes);
    return v;
}
template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
void KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::erase(std::size_t idx)
{
    auto p = _nodes.begin();
    std::advance(p, idx);
    _nodes.erase(p);
    construct(_nodes);
}

template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
std::size_t KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::nn(const Sample & p) const {
    auto n = gnnn_search(p, 1);
    return n[0];
}
template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
std::vector<std::size_t> KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::knn(const Sample& p, std::size_t K) const
{
    return gnnn_search(p, K);
}

template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
std::vector<std::pair<std::size_t, typename Distance::distance_type>>
KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::rnn(const Sample& query,
                                                                  typename Distance::distance_type threshold) const
{
    std::vector<std::pair<size_t, typename Distance::distance_type>> result;

    // variables for choosen nodes during search
    std::vector<distance_value_type_t> choosen_distances;
    std::vector<int> choosen_nodes;

    // temp variables
    std::vector<distance_value_type_t> distances;
    distance_value_type_t distance;

    Distance distancer;

    auto num_expansions = _nodes.size();

    auto num_greedy_moves  = round(_nodes.size() * 0.2);
    int iterations = 10;
    //

    if (_nodes.size() > 0) {
        for (int i = 0; i < iterations; i++) {

            std::random_device rnd;
            std::mt19937 mt(rnd());
            std::uniform_int_distribution<int> dist(0, _nodes.size() - 1);

            // get initial random node from the graph
            int checking_node = dist(mt);
            int prev_node = -1;
            int new_node;

            // walk from initial node on distance 'num_greedy_moves' steps
            for (int j = 0; j < num_greedy_moves; j++) {
                distances.clear();
                // 0 index is for node itself, 1 - is first circle of neighbours
                auto neighbours = this->getNeighbours(checking_node, 1)[1];

                // get first num_expansions neighbours for the checking node and calculate distances to the query
                for (int p = 0; p < num_expansions; p++) {
                    if (p < neighbours.size()) {
                        distance = distancer(_nodes[neighbours[p]], query);
                        distances.push_back(distance);

                        if (std::find(choosen_nodes.begin(), choosen_nodes.end(), neighbours[p])
                            == choosen_nodes.end()) {
                            choosen_distances.push_back(distance);
                            choosen_nodes.push_back(neighbours[p]);
                        }
                    }
                }

                auto min_index = std::min_element(distances.begin(), distances.end());
                new_node = neighbours[std::distance(distances.begin(), min_index)];
                // if we back to the visited node then we fall in loop and search is complete
                if (new_node == prev_node) {
                    break;
                }
                prev_node = checking_node;
                checking_node = new_node;
            }
        }

        // sort distances and return corresopnding nodes from choosen
        auto idxs = sort_indexes(choosen_distances);
        for (std::size_t i = 0; i < idxs.size(); i++) {
            if (choosen_distances[idxs[i]] <= threshold) {
                result.push_back(std::pair{choosen_nodes[idxs[i]], choosen_distances[idxs[i]]});
            } else {
                break;
            }
        }
    }
    return result;
}
}
#endif
