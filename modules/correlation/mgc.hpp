/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2018 PANDA Team
*/

/*
MGC - Multiscale Graph Correlation
a correlation coefficient with finds nonlinear dependencies in data sets and is optimized for small sample sizes.

Multiscale Graph Correlation, Bridgeford, Eric W and Shen, Censheng and Wang, Shangsi and Vogelstein, Joshua,
2018, doi = 10.5281/ZENODO.1246967


Copyright (c) 2018, Michael Welsch
*/

#ifndef _METRIC_CORRELATION_DETAILS_MGC_HPP
#define _METRIC_CORRELATION_DETAILS_MGC_HPP

#include "../../3rdparty/blaze/Math.h"

namespace metric {

template <typename T>
using DistanceMatrix = blaze::SymmetricMatrix<blaze::DynamicMatrix<T>>;

/** @class MGC
 *  @brief Multiscale graph correlation
 *  @tparam recType1 type of the left hand input
 *  @tparam Metric1  type of metric associated with recType1
 *  @tparam recType2  type of the right hand input
 *  @tparam Metric2 type of metric associated with recType2
 */
template <class recType1, class Metric1, class recType2, class Metric2>
struct MGC {
    /** @brief return correlation betweeen a and b
     * @param a container of values of type recType1
     * @param b container of values of type recType2
     * @return correlation betwen a and b
     */
    template <typename Container1, typename Container2>
    double operator()(const Container1& a, const Container2& b) const;

    /** @brief return estimate of the correlation betweeen a and b
     * @param a container of values of type recType1
     * @param b container of values of type recType2
     * @param sampleSize
     * @param threshold
     * @param maxIterations
     * @return estimate of the correlation betwen a and b
     */
    template <typename Container1, typename Container2>
    double estimate(const Container1& a, const Container2& b, const size_t sampleSize = 250,
        const double threshold = 0.05, size_t maxIterations = 1000);

    template <typename Container1, typename Container2>
    std::vector<double> correlation(const Container1& a, const Container2& b, const int from, const int to) const;

    /**
     * @brief
     *
     * @param data
     * @return
     */
    double mean(const std::vector<double>& data);

    /**
     * @brief
     *
     * @param data
     * @param mean
     * @return double
     */
    double variance(const std::vector<double>& data, const double mean);

    /**
     * @brief
     *
     * @param prob
     * @param mu
     * @param sigma
     * @return
     */
    std::vector<double> icdf(const std::vector<double>& prob, const double mu, const double sigma);

    /**
     * @brief
     *
     * @param z
     * @return
     */
    double erfcinv(const double z);

    /**
     * @brief
     *
     * @param p
     * @param q
     * @return
     */
    double erfinv_imp(const double p, const double q);

    /**
     * @brief
     *
     * @param poly
     * @param z
     * @return
     */
    double polyeval(const std::vector<double>& poly, const double z);
    /**
     * @brief
     *
     * @param data
     * @return
     */
    double peak2ems(const std::vector<double>& data);

    /**
     * @brief
     *
     * @param a
     * @param b
     * @param n
     * @return
     */
    std::vector<double> linspace(double a, double b, int n);
};

/**
 * @class MGC_direct
 * @brief
 *
 */
struct MGC_direct {
    /**
     * @brief
     *
     * @tparam T value type of input
     * @param a distance matrix
     * @param b distance matrix
     * @return sample MGC statistic within [-1,1]
     */
    template <typename T>
    T operator()(const DistanceMatrix<T>& a, const DistanceMatrix<T>& b);

    /**
     * @brief Computes the centered distance matrix
     *
     * @tparam T value type of input matrix
     * @param X distance matrix
     * @return   centered distance matrix
     */
    template <typename T>
    blaze::DynamicMatrix<T> center_distance_matrix(const DistanceMatrix<T>& X);

    /**
     * @brief
     *
     * @tparam T value type of input matrix
     * @param data distance matrix
     * @return
     */
    template <typename T>
    blaze::DynamicMatrix<size_t> rank_distance_matrix(const DistanceMatrix<T>& data);

    /**
     * @brief Computes the ranked centered distance matrix
     *
     * @tparam T value type of input matrix
     * @param X distance matrix
     * @return  ranked centered distance matrix
     */
    template <typename T>
    blaze::DynamicMatrix<size_t> center_ranked_distance_matrix(const DistanceMatrix<T>& X);

    /**
     * @brief Computes all local correlations
     *
     * @tparam T value type of input matrices
     * @param A properly transformed distance matrix;
     * @param B properly transformed distance matrix;
     * @param RX column-ranking matrix of A
     * @param RY column-ranking matrix of B
     * @return all local covariances matrix
     */
    template <typename T>
    blaze::DynamicMatrix<T> local_covariance(const blaze::DynamicMatrix<T>& A, const blaze::DynamicMatrix<T>& B,
        const blaze::DynamicMatrix<size_t>& RX, const blaze::DynamicMatrix<size_t>& RY);

    /**
     * @brief
     *
     * @param corr[out]
     * @param varX [in]
     * @param varY [in]
     */
    template <typename T>
    void normalize_generalized_correlation(
        blaze::DynamicMatrix<T>& corr, const blaze::DynamicMatrix<T>& varX, const blaze::DynamicMatrix<T>& varY);

    /**
     * @brief
     *
     * @param t
     * @return
     */
    template <typename T>
    T rational_approximation(const T t);

    /**
     * @brief
     *
     * @param p
     * @return
     */
    template <typename T>
    T normal_CDF_inverse(const T p);

    /**
     * @brief
     *
     * @param p
     * @return
     */
    template <typename T>
    T icdf_normal(const T p);

    /**
     * @brief Finds a grid of significance in the local correlation matrix by thresholding.
     *
     * @tparam T value type
     * @param localCorr all local correlations
     * @param p  sample size of original data (which may not equal m or n in case of repeating data).
     * @return binary matrix of size m and n, with 1's indicating the significant region
     */
    template <typename T>
    blaze::DynamicMatrix<bool> significant_local_correlation(const blaze::DynamicMatrix<T>& localCorr, T p = 0.02);

    /**
     * @brief calculate Frobenius norm of given matrix
     *
     * @tparam T value type of input matrx
     * @param matrix arbitrary matrix
     * @return Frobenius norm of given matrix
     */
    template <typename T>
    T frobeniusNorm(const blaze::DynamicMatrix<T>& matrix);

    /**
     * @brief
     *
     * @tparam T value type
     * @param m1
     * @param m2
     * @return
     */
    template <typename T>
    T max_in_matrix_regarding_second_boolean_matrix(
        const blaze::DynamicMatrix<T>& m1, const blaze::DynamicMatrix<bool>& m2);

    /**
     * @brief Finds the maximal scale within the significant grid that is represented by a boolean matrix R,
     * @details If the area of R is too small, it return the localCorr.
     *
     * @tparam T
     * @param corr matric of all local correlations
     * @param R binary matrix that indicating the significant region of corr´(same size like corr)
     * @return sample MGC statistic within [-1,1]
     */
    template <typename T>
    T optimal_local_generalized_correlation(const blaze::DynamicMatrix<T>& corr, const blaze::DynamicMatrix<bool>& R);
};

}  // namespace metric

#include "mgc.cpp"

#endif  // header guard
