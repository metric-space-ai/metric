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

#include <cstddef>

#include <metric/numeric.hpp>

namespace mtrc {

template <typename T> using DistanceMatrix = mtrc::numeric::SymmetricMatrix<mtrc::numeric::DynamicMatrix<T>>;

struct mgc_resource_options {
	std::size_t max_exact_records = 4096;
	std::size_t max_distance_evaluations = 100'000'000;
	std::size_t max_matrix_cells = 100'000'000;
};

/** @class MGC
 *  @brief Multiscale graph correlation
 *  @details A dependence/correlation statistic between two paired finite metric
 *  spaces, NOT a metric. operator() returns a single sample MGC statistic in
 *  [-1, 1] (~1 strong dependence, ~0 none); it does not compute a p-value or
 *  significance test. Inputs must be paired and of equal size.
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
	explicit MGC(const Metric1 &m1 = Metric1(), const Metric2 &m2 = Metric2(),
				 mgc_resource_options resource_options = {})
		: metric1(m1), metric2(m2), resource_options(resource_options)
	{
	}

	/** @brief sample MGC dependence statistic between paired spaces a and b
	 * @details A dependence/correlation statistic in [-1, 1], NOT a metric or distance.
	 * @param a container of values of type RecType1
	 * @param b container of values of type RecType2
	 * @return sample MGC statistic in [-1, 1] (no p-value); a and b must be paired and equal-sized
	 */
	template <typename Container1, typename Container2>
	double operator()(const Container1 &a, const Container2 &b) const;

	/** @brief subsampled estimate of the MGC dependence statistic between a and b
	 * @details Same dependence/correlation semantics as operator() (a value in [-1, 1],
	 * NOT a metric); trades exactness for speed on large inputs.
	 * @param a container of values of type RecType1
	 * @param b container of values of type RecType2
	 * @param sampleSize
	 * @param threshold
	 * @param maxIterations
	 * @return estimated sample MGC statistic in [-1, 1]
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
	mgc_resource_options resource_options;
};

/**
 * @class MGC_direct
 * @brief
 *
 */
struct MGC_direct {
	explicit MGC_direct(mgc_resource_options resource_options = {}) : resource_options(resource_options) {}

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
	template <typename T> mtrc::numeric::DynamicMatrix<T> center_distance_matrix(const DistanceMatrix<T> &X);

	/**
	 * @brief
	 *
	 * @tparam T value type of input matrix
	 * @param data distance matrix
	 * @return
	 */
	template <typename T> mtrc::numeric::DynamicMatrix<size_t> rank_distance_matrix(const DistanceMatrix<T> &data);

	/**
	 * @brief Computes the ranked centered distance matrix
	 *
	 * @tparam T value type of input matrix
	 * @param X distance matrix
	 * @return  ranked centered distance matrix
	 */
	template <typename T>
	mtrc::numeric::DynamicMatrix<size_t> center_ranked_distance_matrix(const DistanceMatrix<T> &X);

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
	mtrc::numeric::DynamicMatrix<T> local_covariance(const mtrc::numeric::DynamicMatrix<T> &A,
													   const mtrc::numeric::DynamicMatrix<T> &B,
													   const mtrc::numeric::DynamicMatrix<size_t> &RX,
													   const mtrc::numeric::DynamicMatrix<size_t> &RY);

	/**
	 * @brief
	 *
	 * @param corr[out]
	 * @param varX [in]
	 * @param varY [in]
	 */
	template <typename T>
	void normalize_generalized_correlation(mtrc::numeric::DynamicMatrix<T> &corr,
										   const mtrc::numeric::DynamicMatrix<T> &varX,
										   const mtrc::numeric::DynamicMatrix<T> &varY);

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
	mtrc::numeric::DynamicMatrix<bool>
	significant_local_correlation(const mtrc::numeric::DynamicMatrix<T> &localCorr, T p = 0.02);

	/**
	 * @brief calculate Frobenius norm of given matrix
	 *
	 * @tparam T value type of input matrx
	 * @param matrix arbitrary matrix
	 * @return Frobenius norm of given matrix
	 */
	template <typename T> T frobeniusNorm(const mtrc::numeric::DynamicMatrix<T> &matrix);

	/**
	 * @brief
	 *
	 * @tparam T value type
	 * @param m1
	 * @param m2
	 * @return
	 */
	template <typename T>
	T max_in_matrix_regarding_second_boolean_matrix(const mtrc::numeric::DynamicMatrix<T> &m1,
													const mtrc::numeric::DynamicMatrix<bool> &m2);

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
	T optimal_local_generalized_correlation(const mtrc::numeric::DynamicMatrix<T> &corr,
											const mtrc::numeric::DynamicMatrix<bool> &R);

  private:
	mgc_resource_options resource_options;
};

} // namespace mtrc

#include "mgc.cpp"

#endif // header guard
