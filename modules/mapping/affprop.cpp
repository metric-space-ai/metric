/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 M.Welsch
*/
#include "affprop.hpp"

#include "blaze/Math.h"
#include "../distance/k-related/Standards.hpp"

#include <vector>
#include <string>
#include <tuple>
#include <cassert>

namespace metric {

namespace affprop_details {

    //build similarity matrix
    template <typename T, typename DistanceMatrix>
    blaze::SymmetricMatrix<blaze::DynamicMatrix<T, blaze::rowMajor>> similarity_matrix(
        const DistanceMatrix& DM, const T preference)
    {
        blaze::SymmetricMatrix<blaze::DynamicMatrix<T, blaze::rowMajor>> SM(DM.size());
        T pmin = 0;
        T pmax = -2e21;
        for (std::size_t i = 0; i < DM.size(); ++i) {
            for (std::size_t j = i; j < DM.size(); ++j) {
                T similarity = -DM(i, j);
                if (similarity < pmin)
                    pmin = similarity;
                if (similarity > pmax)
                    pmax = similarity;
                SM(i, j) = similarity;
            }
        }

        for (std::size_t i = 0; i < SM.columns(); i++) {
            SM(i, i) = preference * pmax + (1 - preference) * pmin;
        }
        return SM;
    }

    template <typename T, typename SymetricMatrix>
    T update_responsibilities(blaze::DynamicMatrix<T, blaze::rowMajor>& R, const SymetricMatrix& S,
        const blaze::DynamicMatrix<T, blaze::rowMajor>& A, const T& damp)
    {
        auto n = S.rows();
        T maxabs = 0;

        std::vector<std::size_t> I1(n);  // I1[i] is the column index of the maximum element in (A+S) vector
        std::vector<T> Y1(n);  // Y1[i] is the maximum element in (A+S) vector
        std::vector<T> Y2(n);  // Y2[i] is the second maximum element in (A+S) vector

        // Find the first and second maximum elements along each row
        for (std::size_t i = 0; i < n; ++i) {
            T v1 = A(i, 0) + S(i, 0);
            T v2 = A(i, 1) + S(i, 1);
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
        for (std::size_t j = 2; j < n; ++j) {
            for (std::size_t i = 0; i < n; ++i) {
                T v = A(i, j) + S(i, j);
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
        for (std::size_t j = 0; j < n; ++j) {
            for (std::size_t i = 0; i < n; ++i) {
                T Rij_old = R(i, j);
                T mv = (j == I1[i] ? Y2[i] : Y1[i]);
                T Rij_new = S(i, j) - mv;

                // update
                R(i, j) = damp * Rij_old + (1 - damp) * Rij_new;

                // compute convergenze criteria
                T abs_ij = std::abs(Rij_old - Rij_new);
                if (abs_ij > maxabs)
                    maxabs = abs_ij;
            }
        }

        return maxabs;
    }

    // compute availabilities
    template <typename T, typename Matrix>
    T update_availabilities(Matrix& A, const Matrix& R, const T& damp)
    {
        auto n = R.rows();
        T maxabs = 0;
        for (std::size_t j = 0; j < n; ++j) {
            T rjj = R(j, j);

            // compute sum
            T sum = 0;
            for (std::size_t i = 0; i < n; ++i) {
                if (i != j) {
                    T r = R(i, j);
                    if (r > 0)
                        sum += r;
                }
            }

            for (std::size_t i = 0; i < n; ++i) {
                T Aij_old = A(i, j);
                T Aij_new;
                if (i == j)
                    Aij_new = sum;
                else {
                    T r = R(i, j);
                    T u = rjj + sum;
                    if (r > 0)
                        u -= r;
                    Aij_new = (u < 0 ? u : 0);
                }

                // update
                A(i, j) = damp * Aij_old + (1 - damp) * Aij_new;

                // compute convergenze criteria
                T abs_ij = std::abs(Aij_old - Aij_new);
                if (abs_ij > maxabs)
                    maxabs = abs_ij;
            }
        }
        return maxabs;
    }

    // extract all exemplars
    template <typename T, typename Matrix>
    std::vector<std::size_t> extract_exemplars(const Matrix& A, const Matrix& R)
    {
        auto n = A.rows();
        std::vector<std::size_t> r;
        for (std::size_t i = 0; i < n; ++i) {
            if (A(i, i) + R(i, i) > 0)
                r.push_back(i);
        }
        return r;
    }

    // get assignments
    template <typename T, typename SymmetricMatrix>
    std::tuple<std::vector<std::size_t>, std::vector<std::size_t>> get_assignments(const SymmetricMatrix& S,
                                                                                   const std::vector<std::size_t>& exemplars)
    {
        auto n = S.rows();
        auto k = exemplars.size();
        // TODO: check performance of lambda
        auto extractor = [&S, &exemplars](auto i, auto j) { return S(exemplars[i], j); };
        std::vector<std::size_t> a(n);
        std::vector<std::size_t> cnts(k, 0);

        for (std::size_t j = 0; j < n; ++j) {
            std::size_t p = 0;
            T v = extractor(0, j);  //Se[0][j];
            for (std::size_t i = 1; i < k; ++i) {
                T s = extractor(i, j);  //Se[i][j];

                if (s > v) {
                    v = s;
                    p = i;
                }
            }
            a[j] = p;
        }

        for (std::size_t i = 0; i < k; ++i) {
            a[exemplars[i]] = i;
        }

        for (std::size_t i = 0; i < n; ++i) {
            cnts[a[i]] += 1;
        }

        return { a, cnts };
    }

}  // end namespace affprop_details

/**
 * @brief
 *
 * @param data distance matrix
 * @param preference
 * @param maxiter
 * @param tol
 * @param damp
 * @return
*/
template <typename RecType, typename Metric>
auto AffProp<RecType, Metric>::operator()(const Matrix<RecType, Metric>& DM) const
    -> std::tuple<std::vector<std::size_t>, std::vector<std::size_t>, std::vector<std::size_t>>
{
    // check arguments
    auto n = DM.size();
    assert(n >= 2);  //the number of samples must be at least 2.
    assert(tol > 0);  //tol must be a positive value.
    assert(0 <= damp && damp < 1);  // damp must be between 0 and 1.
    assert(0 <= preference && preference < 1);  // preference must be between 0 and 1.

    // build similarity matrix with preference
    auto S = metric::affprop_details::similarity_matrix(DM, preference);
    // initialize messages
    blaze::DynamicMatrix<Value, blaze::rowMajor> R(n, n);
    blaze::DynamicMatrix<Value, blaze::rowMajor> A(n, n);
    // main loop
    int t = 0;
    bool isConverged = false;
    while (!isConverged && t < maxiter) {
        t += 1;

        // compute new messages
        Value maxabsR = metric::affprop_details::update_responsibilities(R, S, A, damp);
        Value maxabsA = metric::affprop_details::update_availabilities(A, R, damp);

        // determine convergence
        Value ch = std::max(maxabsA, maxabsR) / (1 - damp);
        isConverged = (ch < tol);
    }
    // extract exemplars and assignments
    auto exemplars = metric::affprop_details::extract_exemplars<Value>(A, R);
    auto [assignments, counts] = metric::affprop_details::get_assignments<Value>(S, exemplars);

    return { assignments, exemplars, counts };
}

} // metric
