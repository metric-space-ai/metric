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

#include <blaze/Blaze.h>

namespace metric {

template <typename T> using DistanceMatrix = blaze::SymmetricMatrix<blaze::DynamicMatrix<T>>;

/** @class MGC
 *  @brief Multiscale graph correlation
 *  @tparam RecType1 type of the left hand input
 *  @tparam Metric1  type of metric associated with RecType1
 *  @tparam RecType2  type of the right hand input
 *  @tparam Metric2 type of metric associated with RecType2
 */
template <class RecType1, class Metric1, class RecType2, class Metric2> class MGC {
  public:
	/**
	 * @brief Construct MGC object
	 *
	 * @param m1 Metric1 object
	 * @param m2 Metric1 object
	 */
	explicit MGC(const Metric1 &m1 = Metric1(), const Metric2 &m2 = Metric2()) : metric1(m1), metric2(m2) {}

	/** @brief return correlation betweeen a and b
	 * @param a container of values of type RecType1
	 * @param b container of values of type RecType2
	 * @return correlation betwen a and b
	 */
	template <typename Container1, typename Container2>
	double operator()(const Container1 &a, const Container2 &b) const;

	/** @brief return estimate of the correlation betweeen a and b
	 * @param a container of values of type RecType1
	 * @param b container of values of type RecType2
	 * @param sampleSize
	 * @param threshold
	 * @param maxIterations
	 * @return estimate of the correlation betwen a and b
	 */
	template <typename Container1, typename Container2>
	double estimate(const Container1 &a, const Container2 &b, const size_t sampleSize = 250,
					const double threshold = 0.05, size_t maxIterations = 1000) const;

	/** @brief return vector of mgc values calculated for different data shifts
	 * @param a container of values of type RecType1
	 * @param b container of values of type RecType2
	 * @param n number of delayed computations in +/- direction
	 * @return vector of mgc values calculated for different data shifts
	 */
	template <typename Container1, typename Container2>
	std::vector<double> xcorr(const Container1 &a, const Container2 &b, const int n) const;

	/**
	 * @brief return distance matrix
	 *
	 * @param c container of values
	 * @return distance matrix
	 */
	template <typename Container, typename Metric>
	DistanceMatrix<double> computeDistanceMatrix(const Container &c, const Metric &metric) const;

  private:
	Metric1 metric1;
	Metric2 metric2;
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
	template <typename T> T operator()(const DistanceMatrix<T> &a, const DistanceMatrix<T> &b);

	/** @brief return vector of mgc values calculated for different data shifts
	 * @param a distance matrix
	 * @param b distance matrix
	 * @param n number of delayed computations in +/- direction
	 * @return vector of mgc values calculated for different data shifts
	 */
	template <typename T>
	std::vector<double> xcorr(const DistanceMatrix<T> &a, const DistanceMatrix<T> &b, const unsigned int n);

	/**
	 * @brief Computes the centered distance matrix
	 *
	 * @tparam T value type of input matrix
	 * @param X distance matrix
	 * @return   centered distance matrix
	 */
	template <typename T> blaze::DynamicMatrix<T> center_distance_matrix(const DistanceMatrix<T> &X);

	/**
	 * @brief
	 *
	 * @tparam T value type of input matrix
	 * @param data distance matrix
	 * @return
	 */
	template <typename T> blaze::DynamicMatrix<size_t> rank_distance_matrix(const DistanceMatrix<T> &data);

	/**
	 * @brief Computes the ranked centered distance matrix
	 *
	 * @tparam T value type of input matrix
	 * @param X distance matrix
	 * @return  ranked centered distance matrix
	 */
	template <typename T> blaze::DynamicMatrix<size_t> center_ranked_distance_matrix(const DistanceMatrix<T> &X);

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
	blaze::DynamicMatrix<T> local_covariance(const blaze::DynamicMatrix<T> &A, const blaze::DynamicMatrix<T> &B,
											 const blaze::DynamicMatrix<size_t> &RX,
											 const blaze::DynamicMatrix<size_t> &RY);

	/**
	 * @brief
	 *
	 * @param corr[out]
	 * @param varX [in]
	 * @param varY [in]
	 */
	template <typename T>
	void normalize_generalized_correlation(blaze::DynamicMatrix<T> &corr, const blaze::DynamicMatrix<T> &varX,
										   const blaze::DynamicMatrix<T> &varY);

	/**
	 * @brief
	 *
	 * @param t
	 * @return
	 */
	template <typename T> T rational_approximation(const T t);

	/**
	 * @brief
	 *
	 * @param p
	 * @return
	 */
	template <typename T> T normal_CDF_inverse(const T p);

	/**
	 * @brief
	 *
	 * @param p
	 * @return
	 */
	template <typename T> T icdf_normal(const T p);

	/**
	 * @brief Finds a grid of significance in the local correlation matrix by thresholding.
	 *
	 * @tparam T value type
	 * @param localCorr all local correlations
	 * @param p  sample size of original data (which may not equal m or n in case of repeating data).
	 * @return binary matrix of size m and n, with 1's indicating the significant region
	 */
	template <typename T>
	blaze::DynamicMatrix<bool> significant_local_correlation(const blaze::DynamicMatrix<T> &localCorr, T p = 0.02);

	/**
	 * @brief calculate Frobenius norm of given matrix
	 *
	 * @tparam T value type of input matrx
	 * @param matrix arbitrary matrix
	 * @return Frobenius norm of given matrix
	 */
	template <typename T> T frobeniusNorm(const blaze::DynamicMatrix<T> &matrix);

	/**
	 * @brief
	 *
	 * @tparam T value type
	 * @param m1
	 * @param m2
	 * @return
	 */
	template <typename T>
	T max_in_matrix_regarding_second_boolean_matrix(const blaze::DynamicMatrix<T> &m1,
													const blaze::DynamicMatrix<bool> &m2);

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
	T optimal_local_generalized_correlation(const blaze::DynamicMatrix<T> &corr, const blaze::DynamicMatrix<bool> &R);
};

} // namespace metric

#include "mgc.cpp"

#endif // header guard
