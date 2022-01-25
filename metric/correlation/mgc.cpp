/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2018 Michael Welsch
*/
#include "mgc.hpp"

#include <algorithm>
#include <assert.h>
#include <complex>
#include <functional>
#include <iterator>
#include <limits>
#include <numeric>
#include <vector>

#if defined(_MSC_VER)
/* Microsoft C/C++-compatible compiler */
#include <intrin.h>
#elif defined(GNUC) && (defined(x86_64) || defined(i386))
/* GCC-compatible compiler, targeting x86/x86-64 */

#include <x86intrin.h>
#elif defined(GNUC) && defined(ARM_NEON)
/* GCC-compatible compiler, targeting ARM with NEON */

#include <arm_neon.h>
#elif defined(GNUC) && defined(IWMMXT)
/* GCC-compatible compiler, targeting ARM with WMMX */

#include <mmintrin.h>
#elif (defined(GNUC) || defined(xlC)) && (defined(VEC) || defined(ALTIVEC))
/* XLC or GCC-compatible compiler, targeting PowerPC with VMX/VSX */

#include <altivec.h>
#elif defined(GNUC) && defined(SPE)
/* GCC-compatible compiler, targeting PowerPC with SPE */

#include <spe.h>
#endif

#include <math.h>
#include <random>

#include <chrono>

#include <blaze/Math.h>

#include "../distance.hpp"
#include "../utils/graph/connected_components.hpp"
#include "estimator_helpers.hpp"

namespace metric {

// computes the (pairwise) distance matrix for arbitrary random access matrix like containers.
template <typename Container> Container distance_matrix(const Container &data)
{
	typedef typename Container::value_type Row;
	typedef typename Row::value_type T;
	Container matrix(data.size(), Row(data.size())); // initialize

	auto distance_function = metric::Euclidean<T>();
	for (size_t i = 0; i < data.size(); ++i) {
		matrix[i][i] = 0;
		for (size_t j = i + 1; j < data.size(); ++j) {
			T distance = distance_function(data[i], data[j]);
			matrix[i][j] = distance;
			matrix[j][i] = distance;
		}
	}
	return matrix;
}

template <typename T> blaze::DynamicMatrix<size_t> MGC_direct::rank_distance_matrix(const DistanceMatrix<T> &data)
{
	blaze::DynamicMatrix<size_t> matrix(data.rows(), data.columns());

	std::vector<size_t> indexes(data.rows());
	std::iota(indexes.begin(), indexes.end(), 0);

	for (int i = 0; i < data.rows(); ++i) {
		auto row = blaze::row(data, i);
		std::sort(indexes.begin(), indexes.end(), [&row](auto i1, auto i2) { return row[i1] < row[i2]; });

		/* Fill result row */
		auto outRow = blaze::row(matrix, i);
		for (size_t iter = 0; iter < row.size(); ++iter) {
			outRow[indexes[iter]] = iter;
		}
	}

	return matrix;
}

template <typename T> blaze::DynamicMatrix<size_t> MGC_direct::center_ranked_distance_matrix(const DistanceMatrix<T> &X)
{
	auto A = rank_distance_matrix(X);

	// blaze::transpose(A);

	return A;
}

template <typename T> blaze::DynamicMatrix<T> MGC_direct::center_distance_matrix(const DistanceMatrix<T> &X)
{
	blaze::DynamicVector<T, blaze::rowVector> list_of_sums = blaze::sum<blaze::columnwise>(X);
	list_of_sums /= X.rows() - 1;

	blaze::DynamicMatrix<T> centered_distance_matrix(X.rows(), X.columns());

	for (auto i = 0; i < X.rows(); ++i) {
		for (auto j = 0; j < X.rows(); ++j) {
			centered_distance_matrix(i, j) = X(i, j) - list_of_sums[j];
		}
	}

	for (int i = 0; i < X.rows(); ++i) {
		centered_distance_matrix(i, i) = 0;
	}

	return centered_distance_matrix;
}

template <typename T>
blaze::DynamicMatrix<T> MGC_direct::local_covariance(const blaze::DynamicMatrix<T> &A, const blaze::DynamicMatrix<T> &B,
													 const blaze::DynamicMatrix<size_t> &RX,
													 const blaze::DynamicMatrix<size_t> &RY)
{
	const size_t n = A.rows();

	const size_t nX = blaze::max(RX) + 1;
	const size_t nY = blaze::max(RY) + 1;

	blaze::DynamicMatrix<T> covXY(nX, nY, 0);
	blaze::DynamicMatrix<T, blaze::columnMajor> EX(nX, 1, 0);
	blaze::DynamicMatrix<T> EY(1, nY, 0);

	// summing up the entrywise product of A and B based on the ranks EX and EY
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j < n; ++j) {

			const auto a = A(i, j);
			const auto b = B(i, j);
			const auto k = RX(i, j);
			const auto l = RY(i, j);
			covXY(k, l) += a * b;
			EX(k, 0) += a;
			EY(0, l) += b;
		}
	}

	for (size_t k = 0; k < nX - 1; ++k) {
		covXY(k + 1, 0) = covXY(k, 0) + covXY(k + 1, 0);
		EX(k + 1, 0) += EX(k, 0);
	}

	blaze::DynamicVector<T, blaze::rowVector> covXY0 = blaze::row(covXY, 0);
	for (size_t l = 0; l < nY - 1; ++l) {
		covXY0[l + 1] += covXY0[l];
		EY(0, l + 1) += EY(0, l);
	}

	for (size_t k = 0; k < nX - 1; ++k) {
		for (size_t l = 0; l < nY - 1; ++l) {
			covXY(k + 1, l + 1) += covXY(k + 1, l) + covXY(k, l + 1) - covXY(k, l);
		}
	}

	covXY -= EX * EY / n / n;

	return covXY;
}

template <typename T> T MGC_direct::rational_approximation(const T t)
{
	// The absolute value of the error should be less than 4.5 e-4.
	std::vector<T> c = {2.515517, 0.802853, 0.010328};
	std::vector<T> d = {1.432788, 0.189269, 0.001308};
	return t - ((c[2] * t + c[1]) * t + c[0]) / (((d[2] * t + d[1]) * t + d[0]) * t + 1.0);
}

template <typename T> T MGC_direct::normal_CDF_inverse(const T p)
{
	if (p < 0.5) {
		return -rational_approximation(std::sqrt(-2.0 * std::log(p))); // F^-1(p) = - G^-1(p)
	} else {
		return rational_approximation(std::sqrt(-2.0 * std::log(1 - p))); // F^-1(p) = G^-1(1-p)
	}
}

template <typename T> T MGC_direct::icdf_normal(const T p) { return normal_CDF_inverse(p); }

template <typename T>
blaze::DynamicMatrix<bool> MGC_direct::significant_local_correlation(const blaze::DynamicMatrix<T> &localCorr, T p)
{
	/* Sample size minus one */
	T sz = T(localCorr.rows() - 1);

	/* Normal approximation, which is equivalent to beta approximation for n larger than 10 */
	T thres = icdf_normal(1 - p / sz) / sqrt(sz * (sz - 3) / 2 - 1);

	/* Take the maximal of threshold and local correlation at the maximal scale */
	thres = std::max(thres, localCorr(localCorr.rows() - 1, localCorr.rows() - 1));

	/* Threshold R = (localCorr > thres) */
	blaze::DynamicMatrix<bool> R;
	R = blaze::map(localCorr, [thres](T e) { return e > thres; });

	auto components = metric::graph::largest_connected_component(R);

	if (components.empty()) {
		return blaze::DynamicMatrix<bool>(R.rows(), R.columns(), false);
	} else {
		return components[0];
	}
}

template <typename T> T MGC_direct::frobeniusNorm(const blaze::DynamicMatrix<T> &matrix)
{
	const auto m = blaze::invsqrt(matrix);
	const double result = blaze::sum(m);

	return std::sqrt(result);
}

template <typename T>
T MGC_direct::max_in_matrix_regarding_second_boolean_matrix(const blaze::DynamicMatrix<T> &m1,
															const blaze::DynamicMatrix<bool> &m2)
{
	return blaze::max(m2 % m1);
}

template <typename T>
T MGC_direct::optimal_local_generalized_correlation(const blaze::DynamicMatrix<T> &corr,
													const blaze::DynamicMatrix<bool> &R)
{
	size_t m = corr.rows();
	size_t n = corr.columns();
	auto MGC = corr(corr.rows() - 1, corr.rows() - 1); // default sample mgc to local corr at maximal scale

	auto optimalScale = m * n; // default the optimal scale to maximal scale

	size_t R_sum = blaze::nonZeros(R);

	if (frobeniusNorm(R) != double(0) && R_sum != R.rows() * R.columns())
	// if (frobeniusNorm(R) != double(0) )
	{

		if (R_sum >= 2 * std::min(m, n)) {
			// proceed only when the region area is sufficiently large
			auto tmp = max_in_matrix_regarding_second_boolean_matrix(corr, R);

			// find all scales within R that maximize the local correlation
			size_t k = 0, l = 0;
			for (size_t i = 0; i < corr.rows(); ++i) {
				// ?
				for (size_t j = 0; j < corr.rows(); ++j) {
					if (corr(i, j) >= tmp && R(i, j) == true) {
						k = i;
						l = j;
					}
				}
			}

			if (tmp >= MGC) {
				MGC = tmp;
				optimalScale = l * m + (k + 1); // take the scale of maximal stat and change to single index
			}
		}
	}

	return MGC;
}

template <typename T>
void MGC_direct::normalize_generalized_correlation(blaze::DynamicMatrix<T> &corr, const blaze::DynamicMatrix<T> &varX,
												   const blaze::DynamicMatrix<T> &varY)
{
	for (size_t i = 0; i < corr.rows(); ++i) {
		for (size_t j = 0; j < corr.rows(); ++j) {
			corr(i, j) = corr(i, j) / std::sqrt(varX(i, i) * varY(j, j));

			if (isnan(corr(i, j))) {
				corr(i, j) = 0;
			} else if (corr(i, j) > 1) {
				corr(i, j) = 1;
			}
		}
	}
}

template <typename T> T MGC_direct::operator()(const DistanceMatrix<T> &X, const DistanceMatrix<T> &Y)
{
	assert(X.rows() == Y.rows());

	// center distance matrix
	blaze::DynamicMatrix<T> A = center_distance_matrix(X);
	blaze::DynamicMatrix<T> B = center_distance_matrix(Y);

	auto RXt = center_ranked_distance_matrix(X);
	auto RYt = center_ranked_distance_matrix(Y);

	// transpose copies
	blaze::DynamicMatrix<T> At = blaze::trans(A);
	blaze::DynamicMatrix<T> Bt = blaze::trans(B);
	blaze::DynamicMatrix<size_t> RX = blaze::trans(RXt);
	blaze::DynamicMatrix<size_t> RY = blaze::trans(RYt);

	// compute generalized correlation
	auto corr = local_covariance(A, Bt, RX, RYt); // compute all local covariances
	auto varX = local_covariance(A, At, RX, RXt); // compute local variances for first data
	auto varY = local_covariance(B, Bt, RY, RYt); // compute local variances for second data

	blaze::clear(A);
	A.shrinkToFit();
	blaze::clear(At);
	At.shrinkToFit();
	blaze::clear(B);
	B.shrinkToFit();
	blaze::clear(Bt);
	Bt.shrinkToFit();
	blaze::clear(RXt);
	RXt.shrinkToFit();
	blaze::clear(RX);
	RX.shrinkToFit();
	blaze::clear(RY);
	RY.shrinkToFit();
	blaze::clear(RYt);
	RYt.shrinkToFit();

	// normalize the generalized correlation
	normalize_generalized_correlation(corr, varX, varY);

	blaze::clear(varX);
	varX.shrinkToFit();
	blaze::clear(varY);
	varY.shrinkToFit();

	/* Find the largest connected region of significant local correlations */
	auto R = significant_local_correlation(corr /*,p=0.02*/);

	/* Find the maximal scaled correlation within the significant region (the Multiscale Graph Correlation) */
	return optimal_local_generalized_correlation(corr, R);
}

template <typename T>
std::vector<double> MGC_direct::xcorr(const DistanceMatrix<T> &a, const DistanceMatrix<T> &b, const unsigned int n)
{
	assert(a.rows() == b.rows());
	assert(n <= std::numeric_limits<int>::max());

	std::vector<double> result;
	result.reserve(2 * n + 1);

	int s = -n;
	if (s <= (int)n) {
		auto g = 9;
	}

	for (int shift = -n; shift <= (int)n; ++shift) {
		DistanceMatrix<T> aShifted;
		DistanceMatrix<T> bShifted;

		const auto start = std::abs(shift);
		const auto length = a.rows() - start;

		if (shift < 0) {
			aShifted = blaze::submatrix(a, start, start, length, length);
			bShifted = blaze::submatrix(b, 0, 0, length, length);
		} else {
			aShifted = blaze::submatrix(a, 0, 0, length, length);
			bShifted = blaze::submatrix(b, start, start, length, length);
		}

		result.push_back(operator()(aShifted, bShifted));
	}

	return result;
}

template <class RecType1, class Metric1, class RecType2, class Metric2>
template <typename Container1, typename Container2>
double MGC<RecType1, Metric1, RecType2, Metric2>::operator()(const Container1 &a, const Container2 &b) const
{
	assert(a.size() == b.size());

	/* Compute distance matrices */
	auto X = computeDistanceMatrix<Container1>(a, metric1);
	auto Y = computeDistanceMatrix<Container2>(b, metric2);

	return MGC_direct()(X, Y);
}

template <class RecType1, class Metric1, class RecType2, class Metric2>
template <typename Container, typename Metric>
DistanceMatrix<double> MGC<RecType1, Metric1, RecType2, Metric2>::computeDistanceMatrix(const Container &c,
																						const Metric &metric) const
{
	DistanceMatrix<double> X(c.size());
	for (size_t i = 0; i < X.rows(); ++i) {
		X(i, i) = 0;
		for (size_t j = i + 1; j < X.columns(); ++j) {
			double distance = metric(c[i], c[j]);
			X(i, j) = distance;
		}
	}

	return X;
}

template <class RecType1, class Metric1, class RecType2, class Metric2>
template <typename Container1, typename Container2>
double MGC<RecType1, Metric1, RecType2, Metric2>::estimate(const Container1 &a, const Container2 &b,
														   const size_t sampleSize, const double threshold,
														   size_t maxIterations) const
{
	assert(a.size() == b.size());

	const size_t dataSize = a.size();

	/* Update maxIterations */
	if (maxIterations == 0) {
		maxIterations = dataSize / sampleSize;
	}

	if (maxIterations > dataSize / sampleSize) {
		maxIterations = dataSize / sampleSize;
	}

	if (maxIterations < 1) {
		return operator()(a, b);
	}

	/* Create shuffle indexes */
	std::vector<size_t> indexes(dataSize);
	std::iota(indexes.begin(), indexes.end(), 0);

	auto rng = std::default_random_engine();
	std::shuffle(indexes.begin(), indexes.end(), rng);

	/* Create vector container for fast random access */
	const std::vector<typename Container1::value_type> vectorA(a.begin(), a.end());
	const std::vector<typename Container2::value_type> vectorB(b.begin(), b.end());

	/* Create samples */
	std::vector<typename Container1::value_type> sampleA;
	std::vector<typename Container2::value_type> sampleB;
	sampleA.reserve(sampleSize);
	sampleB.reserve(sampleSize);

	std::vector<double> mgcValues;
	double mu = 0;
	for (auto i = 1; i <= maxIterations; ++i) {
		size_t start = (i - 1) * sampleSize;
		size_t end = std::min(i * sampleSize - 1, dataSize - 1);

		/* Create samples */
		sampleA.clear();
		sampleB.clear();

		for (auto j = start; j < end; ++j) {
			sampleA.push_back(vectorA[indexes[j]]);
			sampleB.push_back(vectorB[indexes[j]]);
		}

		/* Get sample mgc value */
		double mgc = operator()(sampleA, sampleB);
		mgcValues.push_back(mgc);

		std::sort(mgcValues.begin(), mgcValues.end());

		const size_t n = mgcValues.size();
		const auto p0 = linspace(0.5 / n, 1 - 0.5 / n, n);

		mu = mean(mgcValues);
		double sigma = variance(mgcValues, mu);

		const std::vector<double> synth = icdf(p0, mu, sigma);
		std::vector<double> diff;
		diff.reserve(n);
		for (auto i = 0; i < n; ++i) {
			diff.push_back(mgcValues[i] - synth[i]);
		}

		auto convergence = peak2ems(diff) / n;

		if (convergence < threshold) {
			return mu;
		}
	}

	return mu;
}

template <class RecType1, class Metric1, class RecType2, class Metric2>
template <typename Container1, typename Container2>
std::vector<double> MGC<RecType1, Metric1, RecType2, Metric2>::xcorr(const Container1 &a, const Container2 &b,
																	 const int n) const
{
	assert(a.size() == b.size());

	/* Compute distance matrices */
	auto X = computeDistanceMatrix<Container1>(a, metric1);
	auto Y = computeDistanceMatrix<Container2>(b, metric2);

	return MGC_direct().xcorr(X, Y, n);
}

} // namespace metric
