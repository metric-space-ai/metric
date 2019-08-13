/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 M.Welsch
*/

#ifndef _METRIC_MAPPING_AFFPROP_HPP
#define _METRIC_MAPPING_AFFPROP_HPP

/*
A Affinity Propagation implementation based on a similarity matrix.
*/
//   Reference:
//       Clustering by Passing Messages Between Data Points.
//       Brendan J. Frey and Delbert Dueck
//       Science, vol 315, pages 972-976, 2007.
//

#include <vector>
#include <tuple>

#include "affprop.cpp"
namespace metric {
/**
     * @brief
     *
     * @param data
     * @param preference
     * @param distance_measure
     * @param maxiter
     * @param tol
     * @param damp
     * @return
     */
template <typename T>
std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> affprop(const std::vector<std::vector<T>>& data,
    T preference = 0.5, std::string distance_measure = "euclidian", int maxiter = 200, T tol = 1.0e-6, T damp = 0.5)
{

    // check arguments
    int n = data.size();

    assert(n >= 2);  //the number of samples must be at least 2.
    assert(tol > 0);  //tol must be a positive value.
    assert(0 <= damp && damp < 1);  // damp must be between 0 and 1.
    assert(0 <= preference && preference < 1);  // preference must be between 0 and 1.

    // build similarity matrix with preference
    std::vector<std::vector<T>> S = metric::affprop_details::similarity_matrix(data, preference, distance_measure);

    // initialize messages
    std::vector<std::vector<T>> R(n, std::vector<T>(n, 0));
    std::vector<std::vector<T>> A(n, std::vector<T>(n, 0));

    // main loop
    int t = 0;
    bool isConverged = false;
    while (!isConverged && t < maxiter) {
        t += 1;

        // compute new messages
        T maxabsR = metric::affprop_details::update_responsibilities(R, S, A, damp);
        T maxabsA = metric::affprop_details::update_availabilities(A, R, damp);

        // determine convergence
        T ch = std::max(maxabsA, maxabsR) / (1 - damp);
        isConverged = (ch < tol);
    }
    // extract exemplars and assignments
    auto exemplars = metric::affprop_details::extract_exemplars(A, R);
    auto [assignments, counts] = metric::affprop_details::get_assignments(S, exemplars);

    return { assignments, exemplars, counts };
}

}  // namespace metric

#endif
