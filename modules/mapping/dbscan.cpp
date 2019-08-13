/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 M.Welsch
*/
#ifndef _METRIC_MAPPING_DBSCAN_CPP
#define _METRIC_MAPPING_DBSCAN_CPP
#include <vector>
#include <string>
#include <deque>
#include <numeric>
#include <cassert>
#include "../distance/k-related/Standards.hpp"
namespace metric {

// --------------------------------------------------------------
// DBSCAN
// --------------------------------------------------------------
namespace dbscan_details {
    std::string default_measure(void) { return "euclidian"; }
    template <typename T>
    T distance(const std::vector<T>& a, const std::vector<T>& b, std::string distance_measure)
    {

        assert(a.size() == b.size());  // data vectors have not the same length
        if (distance_measure.compare("euclidian") == 0)
            return metric::Euclidian<T>()(a, b);
        else if (distance_measure.compare("rms") == 0) {
            T val = metric::Euclidian<T>()(a, b);
            return val * val;
        } else if (distance_measure.compare("manhatten") == 0)
            return metric::Manhatten<T>()(a, b);
        else {

            return metric::Euclidian<T>()(a, b);
        }
    }
    // computes the distance matrix (pairwaise)
    template <typename T>
    std::vector<std::vector<T>> distance_matrix(const std::vector<std::vector<T>>& data, std::string distance_measure)
    {

        std::vector<std::vector<T>> matrix(data.size(), std::vector<T>(data.size()));  //initialize
        for (int i = 0; i < data.size(); ++i) {
            for (int j = i; j < data.size(); ++j) {
                T distance = dbscan_details::distance(data[i], data[j], distance_measure);
                matrix[i][j] = distance;
                matrix[j][i] = distance;
            }
        }
        return matrix;
    }

    // key steps
    template <typename T>
    std::deque<int> region_query(std::vector<std::vector<T>> D, int p, T eps)
    {

        std::deque<int> nbs;
        for (int i = 0; i < D.size(); ++i) {
            if (D[p][i] < eps) {
                nbs.push_back(i);
            }
        }
        return nbs;
    }

    // a changing arguments function
    template <typename T>
    int update_cluster(const std::vector<std::vector<T>>& D,  // distance matrix
        const int& k,  // the index of current cluster
        const int& p,  // the index of seeding point
        const T& eps,  // radius of neighborhood
        const int& minpts,  // minimum number of neighbors of a density point
        std::deque<int>& nbs,  // eps-neighborhood of p
        std::vector<int>& assignments,  // assignment vector
        std::vector<bool>& visited)
    {  // visited indicators
        assignments[p] = k;
        int cnt = 1;
        while (!std::empty(nbs)) {
            //q = shift!(nbs)
            int q = nbs[0];
            nbs.pop_front();
            if (!visited[q]) {
                visited[q] = true;
                auto qnbs = region_query(D, q, eps);
                if (qnbs.size() >= minpts) {
                    for (auto x : qnbs) {
                        if (assignments[x] == 0)
                            nbs.push_back(x);
                    }
                }
            }
            if (assignments[q] == 0) {
                assignments[q] = k;
                cnt += 1;
            }
        }
        return cnt;
    }

}  //namespace dbscan_details

// main algorithm
template <typename T>
std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> dbscan(
    const std::vector<std::vector<T>>& data, T eps, int minpts, std::string distance_measure)
{

    // check arguments
    int n = data.size();

    assert(n >= 2);  // error("There must be at least two points.")
    assert(eps > 0);  // error("eps must be a positive real value.")
    assert(minpts >= 1);  // error("minpts must be a positive integer.")

    // build the (pairwaise) distance matrix
    auto D = dbscan_details::distance_matrix(data, distance_measure);

    // initialize
    std::vector<int> seeds;
    std::vector<int> counts;
    std::vector<int> assignments(n, int(0));
    std::vector<bool> visited(n, false);
    std::vector<int> visitseq(n);
    std::iota(visitseq.begin(), visitseq.end(), 0);  // (generates a linear index vector [0, 1, 2, ...])

    // main loop
    int k = 0;
    for (int p : visitseq) {
        if (assignments[p] == 0 && !visited[p]) {
            visited[p] = true;
            auto nbs = dbscan_details::region_query(D, p, eps);
            if (nbs.size() >= minpts) {
                k += 1;
                auto cnt = dbscan_details::update_cluster(D, k, p, eps, minpts, nbs, assignments, visited);
                seeds.push_back(p);
                counts.push_back(cnt);
            }
        }
    }

    // make output
    return { assignments, seeds, counts };
}

}  // namespace metric

#endif
