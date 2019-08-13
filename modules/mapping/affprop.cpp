/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 M.Welsch
*/

#ifndef _METRIC_MAPPING_AFFPROP_CPP
#define _METRIC_MAPPING_AFFPROP_CPP

#include <vector>
#include <string>
#include <tuple>
#include <cassert>

#include "../distance/k-related/Standards.hpp"
namespace metric {
namespace affprop_details {
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

    //build similarity matrix
    template <typename T>
    std::vector<std::vector<T>> similarity_matrix(
        const std::vector<std::vector<T>>& data, const T preference, std::string distance_measure)
    {
        int n = data.size();
        T pmin = 0;
        T pmax = -2e21;
        std::vector<std::vector<T>> matrix(n, std::vector<T>(n));  //initialize
        for (int i = 0; i < n; ++i) {
            for (int j = i; j < n; ++j) {

                T distance = affprop_details::distance(data[i], data[j], distance_measure);
                T similarity = -distance;
                if (similarity < pmin)
                    pmin = similarity;
                if (similarity > pmax)
                    pmax = similarity;
                matrix[i][j] = similarity;
                matrix[j][i] = similarity;
            }
        }

        for (int i = 0; i < n; ++i) {
            matrix[i][i] = preference * pmax + (1 - preference) * pmin;
        }

        return matrix;
    }

    // compute responsibilities
    template <typename T>
    T update_responsibilities(std::vector<std::vector<T>>& R, const std::vector<std::vector<T>>& S,
        const std::vector<std::vector<T>>& A, const T& damp)
    {
        int n = S.size();
        T maxabs = 0;

        std::vector<int> I1(n);  // I1[i] is the column index of the maximum element in (A+S) vector
        std::vector<T> Y1(n);  // Y1[i] is the maximum element in (A+S) vector
        std::vector<T> Y2(n);  // Y2[i] is the second maximum element in (A+S) vector

        // Find the first and second maximum elements along each row
        for (int i = 0; i < n; ++i) {
            T v1 = A[i][0] + S[i][0];
            T v2 = A[i][1] + S[i][1];
            if (v1 > v2) {
                I1[i] = 0;
                Y1[i] = v1;
                Y2[i] = v2;
            } else {
                I1[i] = 1;
                Y1[i] = v2;
                Y2[i] = v1;
            }
        }
        for (int j = 2; j < n; ++j) {
            for (int i = 0; i < n; ++i) {
                T v = A[i][j] + S[i][j];
                if (v > Y2[i]) {
                    if (v > Y1[i]) {
                        Y2[i] = Y1[i];
                        I1[i] = j;
                        Y1[i] = v;
                    } else
                        Y2[i] = v;
                }
            }
        }

        // update R values
        for (int j = 0; j < n; ++j) {
            for (int i = 0; i < n; ++i) {
                T Rij_old = R[i][j];
                T mv = (j == I1[i] ? Y2[i] : Y1[i]);
                T Rij_new = S[i][j] - mv;

                // update
                R[i][j] = damp * Rij_old + (1 - damp) * Rij_new;

                // compute convergenze criteria
                T abs_ij = std::abs(Rij_old - Rij_new);
                if (abs_ij > maxabs)
                    maxabs = abs_ij;
            }
        }

        return maxabs;
    }

    // compute availabilities
    template <typename T>
    T update_availabilities(std::vector<std::vector<T>>& A, const std::vector<std::vector<T>>& R, const T& damp)
    {
        int n = R.size();
        T maxabs = 0;
        for (int j = 0; j < n; ++j) {
            T rjj = R[j][j];

            // compute sum
            T sum = 0;
            for (int i = 0; i < n; ++i) {
                if (i != j) {
                    T r = R[i][j];
                    if (r > 0)
                        sum += r;
                }
            }

            for (int i = 0; i < n; ++i) {
                T Aij_old = A[i][j];
                T Aij_new;
                if (i == j)
                    Aij_new = sum;
                else {
                    T r = R[i][j];
                    T u = rjj + sum;
                    if (r > 0)
                        u -= r;
                    Aij_new = (u < 0 ? u : 0);
                }

                // update
                A[i][j] = damp * Aij_old + (1 - damp) * Aij_new;

                // compute convergenze criteria
                T abs_ij = std::abs(Aij_old - Aij_new);
                if (abs_ij > maxabs)
                    maxabs = abs_ij;
            }
        }
        return maxabs;
    }

    // extract all exemplars
    template <typename T>
    std::vector<int> extract_exemplars(const std::vector<std::vector<T>>& A, const std::vector<std::vector<T>>& R)
    {
        int n = A.size();
        std::vector<int> r;
        for (int i = 0; i < n; ++i) {
            if (A[i][i] + R[i][i] > 0)
                r.push_back(i);
        }
        return r;
    }

    // get assignments
    template <typename T>
    std::tuple<std::vector<int>, std::vector<int>> get_assignments(
        const std::vector<std::vector<T>>& S, const std::vector<int>& exemplars)
    {
        int n = S.size();
        int k = exemplars.size();
        std::vector<std::vector<T>> Se;
        for (int i = 0; i < k; ++i) {
            Se.push_back(S[exemplars[i]]);
        }
        std::vector<int> a(n);
        std::vector<int> cnts(k, 0);

        for (int j = 0; j < n; ++j) {
            int p = 0;
            T v = Se[0][j];
            for (int i = 1; i < k; ++i) {
                T s = Se[i][j];

                if (s > v) {
                    v = s;
                    p = i;
                }
            }
            a[j] = p;
        }

        for (int i = 0; i < k; ++i) {
            a[exemplars[i]] = i;
        }

        for (int i = 0; i < n; ++i) {
            cnts[a[i]] += 1;
        }

        return { a, cnts };
    }

}  // end namespace affprop_details

}

#endif
