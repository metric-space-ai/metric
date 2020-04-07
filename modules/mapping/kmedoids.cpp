/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 M.Welsch
*/

#ifndef _METRIC_MAPPING_KMEDOIDS_CPP
#define _METRIC_MAPPING_KMEDOIDS_CPP

#include "../space/matrix.hpp"
#include <tuple>
#include <vector>
#include <limits>
#include <algorithm>

namespace metric {
namespace kmedoids_details {
    template <typename RecType, typename Metric, typename T = typename metric::Matrix<RecType, Metric>::distType >
    T update_cluster(const metric::Matrix<RecType, Metric>& DM, std::vector<int>& seeds,
        std::vector<int>& assignments, std::vector<int>& sec_nearest, std::vector<int>& counts)
    {

        if (sec_nearest.size() != assignments.size()) {
            sec_nearest.resize(assignments.size());
        }

        // go through and assign each object to nearest medoid, keeping track of total distance.
        T total_distance = 0;
        for (int i = 0; i < assignments.size(); i++) {
            T d1, d2;  // smallest, second smallest distance to medoid, respectively
            int m1, m2;  // index of medoids with distances d1, d2 from object i, respectively
            d1 = d2 = std::numeric_limits<T>::max();
            m1 = m2 = seeds.size();
            for (int m = 0; m < seeds.size(); m++) {
                T d = DM(i, seeds[m]);
                if (d < d1 || seeds[m] == i) {  // prefer the medoid in case of ties.
                    d2 = d1;
                    m2 = m1;
                    d1 = d;
                    m1 = m;
                } else if (d < d2) {
                    d2 = d;
                    m2 = m;
                }
            }
            counts[m1] += 1;
            assignments[i] = m1;
            sec_nearest[i] = m2;
            total_distance += d1;
        }
        return total_distance;
    }

    template <typename RecType, typename Metric>
    void init_medoids(int k, const metric::Matrix<RecType, Metric>& DM, std::vector<int>& seeds,
        std::vector<int>& assignments, std::vector<int>& sec_nearest, std::vector<int>& counts)
    {
        seeds.clear();
        // find first object: object minimum distance to others
        int first_medoid = 0;
        using T = typename metric::Matrix<RecType,Metric>::distType;
        T min_dissim = std::numeric_limits<T>::max();
        for (int i = 0; i < DM.size(); i++) {
            T total = 0;
            for (int j = 0; j < DM.size(); j++) {
                total += DM(i, j);
            }
            if (total < min_dissim) {
                min_dissim = total;
                first_medoid = i;
            }
        }
        // add first object to medoids and compute medoid ids.
        seeds.push_back(first_medoid);
        kmedoids_details::update_cluster(DM, seeds, assignments, sec_nearest, counts);

        // now select next k-1 objects according to KR's BUILD algorithm
        for (int cur_k = 1; cur_k < k; cur_k++) {
            int best_obj = 0;
            T max_gain = 0;
            for (int i = 0; i < DM.size(); i++) {
                if (seeds[assignments[i]] == i)
                    continue;
                T gain = 0;
                for (int j = 0; j < DM.size(); j++) {
                    T DMj = DM(j, seeds[assignments[j]]);  // D from j to its medoid
                    gain += std::max(DMj - DM(i, j), T(0));  // gain from selecting i
                }
                if (gain >= max_gain) {  // set the next medoid to the object that
                    max_gain = gain;  // maximizes the gain function.
                    best_obj = i;
                }
            }

            seeds.push_back(best_obj);
            kmedoids_details::update_cluster(DM, seeds, assignments, sec_nearest, counts);
        }
    }

    template <typename RecType, typename Metric, typename T = typename metric::Matrix<RecType, Metric>::distType>
    T cost(int i, int h, const metric::Matrix<RecType, Metric>& DM, std::vector<int>& seeds,
        std::vector<int>& assignments, std::vector<int>& sec_nearest)
    {
        T total = 0;
        for (int j = 0; j < assignments.size(); j++) {
            int mi = seeds[i];  // object id of medoid i
            T dhj = DM(h, j);  // distance between object h and object j

            int mj1 = seeds[assignments[j]];  // object id of j's nearest medoid
            T dj1 = DM(mj1, j);  // distance to j's nearest medoid

            // check if D bt/w medoid i and j is same as j's current nearest medoid.
            if (DM(mi, j) == dj1) {
                T dj2 = std::numeric_limits<T>::max();
                if (seeds.size() > 1) {  // look at 2nd nearest if there's more than one medoid.
                    int mj2 = seeds[sec_nearest[j]];  // object id of j's 2nd-nearest medoid
                    dj2 = DM(mj2, j);  // D to j's 2nd-nearest medoid
                }
                total += std::min(dj2, dhj) - dj1;

            } else if (dhj < dj1) {
                total += dhj - dj1;
            }
        }
        return total;
    }
}  // namespace kmedoids_details

template <typename RecType, typename Metric, typename T>
std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> kmedoids(
    const metric::Matrix<RecType, Metric>& DM, int k)
{

    // check arguments
    size_t n = DM.size();

    assert(n >= 2);  // error("There must be at least two points.")
    assert(k <= n);  // Attempt to run PAM with more clusters than data.

    // sum up the distance matrix
    T Dsum = 0;
    for (int i = 0; i < DM.size(); ++i) {
        for (int j = i; j < DM.size(); ++j) {
            auto distance = DM(i, j);
            if (i != j)
                Dsum += 2 * distance;
            else
                Dsum += distance;
        }
    }

    std::vector<int> seeds(k);
    std::vector<int> counts(k, 0);
    std::vector<int> assignments(n, 0);
    std::vector<int> sec_nearest(n, 0);  // Index of second closest medoids.  Used by PAM.
    T total_distance;  // Total distance tp their medoid
    T epsilon = 1e-15;  // Normalized sensitivity for convergence

    // set initianl medoids
    kmedoids_details::init_medoids(k, DM, seeds, assignments, sec_nearest, counts);

    T tolerance = epsilon * Dsum / (DM.size() * DM.size());

    while (true) {
        // initial cluster
        for (int i = 0; i < counts.size(); ++i) {
            counts[i] = 0;
        }
        total_distance = kmedoids_details::update_cluster(DM, seeds, assignments, sec_nearest, counts);

        //vars to keep track of minimum
        T minTotalCost = std::numeric_limits<T>::max();
        int minMedoid = 0;
        int minObject = 0;

        //iterate over each medoid
        for (int i = 0; i < k; i++) {
            //iterate over all non-medoids
            for (int h = 0; h < assignments.size(); h++) {
                if (seeds[assignments[h]] == h)
                    continue;

                //see if the total cost of swapping i & h was less than min
                T curCost = kmedoids_details::cost(i, h, DM, seeds, assignments, sec_nearest);
                if (curCost < minTotalCost) {
                    minTotalCost = curCost;
                    minMedoid = i;
                    minObject = h;
                }
            }
        }

        // convergence check
        if (minTotalCost >= -tolerance)
            break;

        // install the new medoid if we found a beneficial swap
        seeds[minMedoid] = minObject;
        assignments[minObject] = minMedoid;
    }

    return { assignments, seeds, counts };
}

// TO DO: dublicate version
template <typename T>
std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> kmedoids_(const std::vector<std::vector<T>>& D, int k,
    int iters  // added by Max Filippov
)
{

    // can be optimized: TODO operate on indices and global dataset instead of copied subset table

    // check arguments
    int n = D.size();

    assert(n >= 2);  // error("There must be at least two points.")
    assert(k <= n);  // Attempt to run PAM with more clusters than data.

    // build the (pairwaise) distance matrix
    T Dsum = 0;  // sum all distances // added by Max Filippov // TODO optimize
    for (auto el1 : D)
        for (auto el2 : el1)
            Dsum += el2;

    std::vector<int> seeds(k);
    std::vector<int> counts(k, 0);
    std::vector<int> assignments(n, 0);
    std::vector<int> sec_nearest(n, 0);  /// Index of second closest medoids.  Used by PAM.
    T total_distance;  /// Total distance tp their medoid
    T epsilon = 1e-15;  /// Normalized sensitivity for convergence

    // set initianl medoids
    metric::Matrix<std::vector<T>, metric::Euclidean<T>> dm(D);
    kmedoids_details::init_medoids(k, dm, seeds, assignments, sec_nearest, counts);

    T tolerance = epsilon * Dsum / (D[0].size() * D.size());

    while (true) {
        iters--;  // added by Max Filippov
        // initial cluster
        for (std::size_t i = 0; i < counts.size(); ++i) {
            counts[i] = 0;
        }
        total_distance = kmedoids_details::update_cluster(dm, seeds, assignments, sec_nearest, counts);

        //vars to keep track of minimum
        T minTotalCost = std::numeric_limits<T>::max();
        int minMedoid = 0;
        int minObject = 0;

        //iterate over each medoid
        for (int i = 0; i < k; i++) {
            //iterate over all non-medoids
            for (std::size_t h = 0; h < assignments.size(); h++) {
                if (static_cast<std::size_t>(seeds[assignments[h]]) == h)
                    continue;

                //see if the total cost of swapping i & h was less than min
                T curCost = kmedoids_details::cost(i, h, dm, seeds, assignments, sec_nearest);
                if (curCost < minTotalCost) {
                    minTotalCost = curCost;
                    minMedoid = i;
                    minObject = h;
                }
            }
        }

        // convergence check
        if (minTotalCost >= -tolerance)
            break;
        if (iters < 0) {
            //std::cout << "\nWarning: exiting kmedoids_ due to exceeding max number of iterations\n";
            break;
        }

        // install the new medoid if we found a beneficial swap
        seeds[minMedoid] = minObject;
        assignments[minObject] = minMedoid;
    }

    return { assignments, seeds, counts };
}

}  // namespace metric

#endif
