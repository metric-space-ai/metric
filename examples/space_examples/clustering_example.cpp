/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Dmitry Vinokurov
*/

#include <vector>
#include <iostream>
#include <algorithm>
#include "metric/space.hpp"

template <typename T>
struct distance {
    int operator()(const T& lhs, const T& rhs) const { return std::abs(lhs - rhs); }
};

template <typename T>
void print_clusters(const ::std::vector<double>& distribution, const std::vector<std::vector<std::size_t>>& clusters, T& tree)
{
    // clusters is a vector of vectors with ID's from tree
    // clusters.size() is always equals to distribution.size()
    // clusters[0].size() == tree.size()*distribution[0]
    // clusters[i].size() == tree.size()*distribution[i] - sum(clusters[0...i-1].size())
    // so number of elements in clusters equal to distribution[distribution.size()-1]*tree.size();

    for (std::size_t v = 0; v < clusters.size(); v++) {
        std::cout << distribution[v] << ": {";
        for (auto& i : clusters[v]) {
            std::cout << "(" << i << ":" << tree[i] << "), ";
        }
        std::cout << " }" << std::endl;
    }
}

int main()
{
	std::cout << "Clustering space example have started" << std::endl;
	std::cout << "" << std::endl;

    metric::Tree<int, distance<int>> tree;
    std::vector<int> data(20);
    std::iota(data.begin(), data.end(), 0);
    tree.insert(data);

    std::cout << tree.size() << std::endl;
    tree.print();
    std::cout << std::endl;
    // distribution is a vector of percents of number of elements
    std::vector<double> distribution = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8 };

    std::vector<int> points = { -1, 0, 1 };

    // points is a arbitrary data.
    // make clusters around points neighbour according to distribution
    auto clusters = tree.clustering(distribution, points);

    print_clusters(distribution, clusters, tree);
    std::cout << std::endl;

    std::vector<std::size_t> ids = { 0, 1, 2 };
    // ids is set of indexes of data array
    // make clusters around neighbour of elements of the data with indexes from ids  according to distribution
    auto clusters2 = tree.clustering(distribution, ids, data);
    print_clusters(distribution, clusters2, tree);
}
