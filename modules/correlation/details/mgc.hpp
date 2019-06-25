/* Panda presents

presents

███╗   ███╗ ██████╗  ██████╗
████╗ ████║██╔════╝ ██╔════╝
██╔████╔██║██║  ███╗██║     
██║╚██╔╝██║██║   ██║██║     
██║ ╚═╝ ██║╚██████╔╝╚██████╗
╚═╝     ╚═╝ ╚═════╝  ╚═════╝
                Michael Welsch (c) 2018.

a library for nonlinear correlation
MGC - Multiscale Graph Correlation
a correlation coefficient with finds nonlinear dependencies in data sets and is optimized for small sample sizes.

Multiscale Graph Correlation, Bridgeford, Eric W and Shen, Censheng and Wang, Shangsi and Vogelstein, Joshua, 
2018, doi = 10.5281/ZENODO.1246967

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018, Michael Welsch
*/

#ifndef _METRIC_CORRELATION_MGC_HPP
#define _METRIC_CORRELATION_MGC_HPP

#include "../../../3rdparty/blaze/Math.h"

namespace metric
{
namespace correlation
{

template<typename T>
using DistanceMatrix = blaze::SymmetricMatrix<blaze::DynamicMatrix<T>>;

// build a mgc_corr function(object) with custom types and metrics
/*  typedef std::vector<double> Rec1;
    typedef std::array<float,3> Rec2;
    typedef metric::mgc_details::generic_euclidian<Rec1> Met1;
    typedef metric::mgc_details::generic_manhatten<Rec2> Met2;

    auto mgc_corr = metric::MGC<Rec1,Met1,Rec2,Met2>();
    auto result = mgc_corr(Container1, Container2);
*/

template <class recType1, class Metric1, class recType2, class Metric2>
struct MGC
{
    template <typename Container1, typename Container2>
    double
    operator()(const Container1 &a, const Container2 &b) const;

	template <typename Container1, typename Container2>
	double estimate(const Container1 &a, const Container2 &b, const size_t sampleSize = 250,
	                const double threshold = 0.05,
	                size_t maxIterations = 1000);

	double mean(const std::vector<double> &data);
	double variance(const std::vector<double> &data, const double mean);
	std::vector<double> icdf(const std::vector<double> &prob, const double mu, const double sigma);
	double erfcinv(const double z);
	double erfinv_imp(const double p, const double q);
	double polyeval(const std::vector<double> &poly, const double z);
	double peak2ems(const std::vector<double> &data);

	std::vector<double> linspace(double a, double b, int n);
};

// input:
//      X distance matrix
//      Y distance matrix
// output:
//      MGC is the sample MGC statistic within [-1,1];
struct MGC_direct
{
	//template <typename T>
	//using Matrix = blaze::DynamicMatrix<T>;

	template <typename T>
	T operator()(const DistanceMatrix<T> &a, const DistanceMatrix<T> &b);

	/* Computes the centered distance matrix */
	template <typename T>
	blaze::DynamicMatrix<T> center_distance_matrix(const DistanceMatrix<T> &X);

	template <typename T>
	blaze::DynamicMatrix<size_t> rank_distance_matrix(const DistanceMatrix<T> &data);

	/* Computes the ranked centered distance matrix */
	template <typename T>
	blaze::DynamicMatrix<size_t> center_ranked_distance_matrix(const DistanceMatrix<T> &X);

	/* Computes all local correlations
	  inputs:
        A is a properly transformed distance matrix;
        B is a properly transformed distance matrix;
        RX is the column-ranking matrix of A;
        RY is the column-ranking matrix of B.
      outputs:
        covXY is the all local covariances matrix.*/
	template <typename T>
	blaze::DynamicMatrix<T> local_covariance(const blaze::DynamicMatrix<T> &A,
	                                         const blaze::DynamicMatrix<T> &B,
	                                         const blaze::DynamicMatrix<size_t> &RX,
	                                         const blaze::DynamicMatrix<size_t> &RY);

	template <typename T>
	void normalize_generalized_correlation(blaze::DynamicMatrix<T> &corr,
											  const blaze::DynamicMatrix<T> &varX,
											  const blaze::DynamicMatrix<T> &varY);

	template <typename T>
	T rational_approximation(const T t);

	template <typename T>
	T normal_CDF_inverse(const T p);

	template <typename T>
	T icdf_normal(const T p);

	/*
Finds a grid of significance in the local correlation matrix by thresholding.
inputs:
	localCorr is all local correlations;
	sz is the sample size of original data (which may not equal m or n in case of repeating data).

outputs:
	R is a binary matrix of size m and n, with 1's indicating the significant region.
*/
	template <typename T>
	blaze::DynamicMatrix<bool> significant_local_correlation(const blaze::DynamicMatrix<T> &localCorr, T p = 0.02);

	template <typename T>
	T frobeniusNorm(const blaze::DynamicMatrix<T> &matrix);

	template <typename T>
	T max_in_matrix_regarding_second_boolean_matrix(const blaze::DynamicMatrix<T> &m1, const blaze::DynamicMatrix<bool> &m2);

	// Finds the maximal scale within the significant grid that is represented by a boolean matrix R:
	// If the area of R is too small, it return the localCorr.
	// input:
	//      localCorr is the matric of all local correlations;
	//      R is a binary matrix that indicating the significant region of localCorr´(same size like localCorr)
	// output:
	//      statMGC is the sample MGC statistic within [-1,1];
	//
	template <typename T>
	T optimal_local_generalized_correlation(const blaze::DynamicMatrix<T> &corr, const blaze::DynamicMatrix<bool> &R);
};

} // end namespace correlation
} // namespace metric

#include "mgc.cpp"

#endif // header guard
