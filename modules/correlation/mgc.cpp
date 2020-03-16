/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2018 Michael Welsch
*/

#include <algorithm>
#include <assert.h>
#include <complex>
#include <functional>
#include <iterator>
#include <numeric>
#include <limits>
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

#include "../../3rdparty/blaze/Math.h"

#include "../utils/graph/connected_components.hpp"
#include "../distance.hpp"
#include "mgc.hpp"


namespace metric {

// computes the (pairwaise) distance matrix for abritrary random acces matrix like containers.
template <typename Container>
Container distance_matrix(const Container& data)
{
    typedef typename Container::value_type Row;
    typedef typename Row::value_type T;
    Container matrix(data.size(), Row(data.size()));  // initialize

    auto distance_function = metric::Euclidian<T>();
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

template <typename T>
blaze::DynamicMatrix<size_t> MGC_direct::rank_distance_matrix(const DistanceMatrix<T>& data)
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

template <typename T>
blaze::DynamicMatrix<size_t> MGC_direct::center_ranked_distance_matrix(const DistanceMatrix<T>& X)
{
    auto A = rank_distance_matrix(X);

    // blaze::transpose(A);

    return A;
}

template <typename T>
blaze::DynamicMatrix<T> MGC_direct::center_distance_matrix(const DistanceMatrix<T>& X)
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
blaze::DynamicMatrix<T> MGC_direct::local_covariance(const blaze::DynamicMatrix<T>& A, const blaze::DynamicMatrix<T>& B,
    const blaze::DynamicMatrix<size_t>& RX, const blaze::DynamicMatrix<size_t>& RY)
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

template <typename T>
T MGC_direct::rational_approximation(const T t)
{
    // The absolute value of the error should be less than 4.5 e-4.
    std::vector<T> c = { 2.515517, 0.802853, 0.010328 };
    std::vector<T> d = { 1.432788, 0.189269, 0.001308 };
    return t - ((c[2] * t + c[1]) * t + c[0]) / (((d[2] * t + d[1]) * t + d[0]) * t + 1.0);
}

template <typename T>
T MGC_direct::normal_CDF_inverse(const T p)
{
    if (p < 0.5) {
        return -rational_approximation(std::sqrt(-2.0 * std::log(p)));  // F^-1(p) = - G^-1(p)
    } else {
        return rational_approximation(std::sqrt(-2.0 * std::log(1 - p)));  // F^-1(p) = G^-1(1-p)
    }
}

template <typename T>
T MGC_direct::icdf_normal(const T p)
{
    return normal_CDF_inverse(p);
}

template <typename T>
blaze::DynamicMatrix<bool> MGC_direct::significant_local_correlation(const blaze::DynamicMatrix<T>& localCorr, T p)
{
    /* Sample size minus one */
    T sz = T(localCorr.rows() - 1);

    /* Normal approximation, which is equivalent to beta approximation for n larger than 10 */
    T thres = icdf_normal(1 - p / sz) / sqrt(sz * (sz - 3) / 2 - 1);

    /* Take the maximal of threshold and local correlation at the maximal scale */
    thres = std::max(thres, localCorr(localCorr.rows() - 1, localCorr.rows() - 1));

    /* Threshold R = (localCorr > thres) */
    blaze::DynamicMatrix<bool> R;
    R = blaze::map(localCorr, [thres](T e) { return e > thres ? true : false; });

    auto components = metric::graph::largest_connected_component(R);

    if (components.empty()) {
        return blaze::DynamicMatrix<bool>(R.rows(), R.columns(), false);
    } else {
        return components[0];
    }
}

template <typename T>
T MGC_direct::frobeniusNorm(const blaze::DynamicMatrix<T>& matrix)
{
    const auto m = blaze::invsqrt(matrix);
    const double result = blaze::sum(m);

    return std::sqrt(result);
}

template <typename T>
T MGC_direct::max_in_matrix_regarding_second_boolean_matrix(
    const blaze::DynamicMatrix<T>& m1, const blaze::DynamicMatrix<bool>& m2)
{
    return blaze::max(m2 % m1);
}

template <typename T>
T MGC_direct::optimal_local_generalized_correlation(
    const blaze::DynamicMatrix<T>& corr, const blaze::DynamicMatrix<bool>& R)
{
    size_t m = corr.rows();
    size_t n = corr.columns();
    auto MGC = corr(corr.rows() - 1, corr.rows() - 1);  // default sample mgc to local corr at maximal scale

    auto optimalScale = m * n;  // default the optimal scale to maximal scale

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
                optimalScale = l * m + (k + 1);  // take the scale of maximal stat and change to single index
            }
        }
    }

    return MGC;
}

template <typename T>
void MGC_direct::normalize_generalized_correlation(
    blaze::DynamicMatrix<T>& corr, const blaze::DynamicMatrix<T>& varX, const blaze::DynamicMatrix<T>& varY)
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

template <typename T>
T MGC_direct::operator()(const DistanceMatrix<T>& X, const DistanceMatrix<T>& Y)
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
    auto corr = local_covariance(A, Bt, RX, RYt);  // compute all local covariances
    auto varX = local_covariance(A, At, RX, RXt);  // compute local variances for first data
    auto varY = local_covariance(B, Bt, RY, RYt);  // compute local variances for second data

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

template<typename T> std::vector<double> MGC_direct::xcorr(const DistanceMatrix<T> &a, const DistanceMatrix<T> &b, const unsigned int n)
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

	template <class recType1, class Metric1, class recType2, class Metric2>
template <typename Container1, typename Container2>
double MGC<recType1, Metric1, recType2, Metric2>::operator()(const Container1& a, const Container2& b) const
{
    assert(a.size() == b.size());

    /* Compute distance matrices */
    auto X = computeDistanceMatrix<Container1>(a, metric1);
    auto Y = computeDistanceMatrix<Container2>(b, metric2);

    return MGC_direct()(X, Y);
}

template <class recType1, class Metric1, class recType2, class Metric2>
template <typename Container, typename Metric>
DistanceMatrix<double> MGC<recType1, Metric1, recType2, Metric2>::computeDistanceMatrix(const Container &c, const Metric & metric) const
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


template <class recType1, class Metric1, class recType2, class Metric2>
template <typename Container1, typename Container2>
double MGC<recType1, Metric1, recType2, Metric2>::estimate(const Container1& a, const Container2& b,
															const size_t sampleSize, const double threshold,
															size_t maxIterations)
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
        std::cout << n << " " << convergence << " " << mgc << " " << mu << std::endl;

        if (convergence < threshold) {
            return mu;
        }
    }

    return mu;
}

template <class recType1, class Metric1, class recType2, class Metric2>
double MGC<recType1, Metric1, recType2, Metric2>::mean(const std::vector<double>& data)
{
    double sum = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        sum += data[i];
    }
    double value = sum / data.size();

    return value;
}

template <class recType1, class Metric1, class recType2, class Metric2>
double MGC<recType1, Metric1, recType2, Metric2>::variance(const std::vector<double>& data, const double mean)
{
    double sum = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        sum += (data[i] - mean) * (data[i] - mean);
    }

    return sum;
}

template <class recType1, class Metric1, class recType2, class Metric2>
std::vector<double> MGC<recType1, Metric1, recType2, Metric2>::icdf(
    const std::vector<double>& prob, const double mu, const double sigma)
{
    std::vector<double> synth;
    synth.reserve(prob.size());
    for (auto p : prob) {
        synth.push_back(mu + -1.41421356237309504880 * erfcinv(2 * p) * sigma);
    }

    return synth;
}

template <class recType1, class Metric1, class recType2, class Metric2>
double MGC<recType1, Metric1, recType2, Metric2>::erfcinv(const double z)
{
    if ((z < 0) || (z > 2))
        std::cout << "Argument outside range [0,2] in inverse erfc function (got p=%1%)." << std::endl;

    double p, q, s;
    if (z > 1) {
        q = 2 - z;
        p = 1 - q;
        s = -1;
    } else {
        p = 1 - z;
        q = z;
        s = 1;
    }
    return s * erfinv_imp(p, q);
}

template <class recType1, class Metric1, class recType2, class Metric2>
double MGC<recType1, Metric1, recType2, Metric2>::erfinv_imp(const double p, const double q)
{
    double result = 0;

    if (p <= 0.5) {
        static const float Y = 0.0891314744949340820313f;
        static const std::vector<double> P = { -0.000508781949658280665617L, -0.00836874819741736770379L,
            0.0334806625409744615033L, -0.0126926147662974029034L, -0.0365637971411762664006L,
            0.0219878681111168899165L, 0.00822687874676915743155L, -0.00538772965071242932965L };
        static const std::vector<double> Q = { 1, -0.970005043303290640362L, -1.56574558234175846809L,
            1.56221558398423026363L, 0.662328840472002992063L, -0.71228902341542847553L, -0.0527396382340099713954L,
            0.0795283687341571680018L, -0.00233393759374190016776L, 0.000886216390456424707504L };

        double g = p * (p + 10);
        double r = polyeval(P, p) / polyeval(Q, p);
        result = g * Y + g * r;
    } else if (q >= 0.25) {
        static const float Y = 2.249481201171875f;
        static const std::vector<double> P = { -0.202433508355938759655L, 0.105264680699391713268L,
            8.37050328343119927838L, 17.6447298408374015486L, -18.8510648058714251895L, -44.6382324441786960818L,
            17.445385985570866523L, 21.1294655448340526258L, -3.67192254707729348546L };
        static const std::vector<double> Q
            = { 1L, 6.24264124854247537712L, 3.9713437953343869095L, -28.6608180499800029974L, -20.1432634680485188801L,
                  48.5609213108739935468L, 10.8268667355460159008L, -22.6436933413139721736L, 1.72114765761200282724L };
        double g = std::sqrt(-2 * std::log(q));
        double xs = q - 0.25;
        double r = polyeval(P, xs) / polyeval(Q, xs);
        result = g / (Y + r);
    } else {
        double x = std::sqrt(-std::log(q));
        if (x < 3) {
            static const float Y = 0.807220458984375f;
            static const std::vector<double> P = { -0.131102781679951906451L, -0.163794047193317060787L,
                0.117030156341995252019L, 0.387079738972604337464L, 0.337785538912035898924L, 0.142869534408157156766L,
                0.0290157910005329060432L, 0.00214558995388805277169L, -0.679465575181126350155e-6L,
                0.285225331782217055858e-7L, -0.681149956853776992068e-9L };
            static const std::vector<double> Q = { 1, 3.46625407242567245975L, 5.38168345707006855425L,
                4.77846592945843778382L, 2.59301921623620271374L, 0.848854343457902036425L, 0.152264338295331783612L,
                0.01105924229346489121L };
            double xs = x - 1.125;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else if (x < 6) {
            static const float Y = 0.93995571136474609375f;
            static const std::vector<double> P
                = { -0.0350353787183177984712L, -0.00222426529213447927281L, 0.0185573306514231072324L,
                      0.00950804701325919603619L, 0.00187123492819559223345L, 0.000157544617424960554631L,
                      0.460469890584317994083e-5L, -0.230404776911882601748e-9L, 0.266339227425782031962e-11L };
            static const std::vector<double> Q
                = { 1L, 1.3653349817554063097L, 0.762059164553623404043L, 0.220091105764131249824L,
                      0.0341589143670947727934L, 0.00263861676657015992959L, 0.764675292302794483503e-4L };
            double xs = x - 3;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else if (x < 18) {
            static const float Y = 0.98362827301025390625f;
            static const std::vector<double> P
                = { -0.0167431005076633737133L, -0.00112951438745580278863L, 0.00105628862152492910091L,
                      0.000209386317487588078668L, 0.149624783758342370182e-4L, 0.449696789927706453732e-6L,
                      0.462596163522878599135e-8L, -0.281128735628831791805e-13L, 0.99055709973310326855e-16L };
            static const std::vector<double> Q
                = { 1L, 0.591429344886417493481L, 0.138151865749083321638L, 0.0160746087093676504695L,
                      0.000964011807005165528527L, 0.275335474764726041141e-4L, 0.282243172016108031869e-6L };
            double xs = x - 6;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else if (x < 44) {
            static const float Y = 0.99714565277099609375f;
            static const std::vector<double> P = { -0.0024978212791898131227L, -0.779190719229053954292e-5L,
                0.254723037413027451751e-4L, 0.162397777342510920873e-5L, 0.396341011304801168516e-7L,
                0.411632831190944208473e-9L, 0.145596286718675035587e-11L, -0.116765012397184275695e-17L };
            static const std::vector<double> Q
                = { 1L, 0.207123112214422517181L, 0.0169410838120975906478L, 0.000690538265622684595676L,
                      0.145007359818232637924e-4L, 0.144437756628144157666e-6L, 0.509761276599778486139e-9L };
            double xs = x - 18;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else {
            static const float Y = 0.99941349029541015625f;
            static const std::vector<double> P = { -0.000539042911019078575891L, -0.28398759004727721098e-6L,
                0.899465114892291446442e-6L, 0.229345859265920864296e-7L, 0.225561444863500149219e-9L,
                0.947846627503022684216e-12L, 0.135880130108924861008e-14L, -0.348890393399948882918e-21L };
            static const std::vector<double> Q
                = { 1L, 0.0845746234001899436914L, 0.00282092984726264681981L, 0.468292921940894236786e-4L,
                      0.399968812193862100054e-6L, 0.161809290887904476097e-8L, 0.231558608310259605225e-11L };
            double xs = x - 44;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        }
    }
    return result;
}

template <class recType1, class Metric1, class recType2, class Metric2>
double MGC<recType1, Metric1, recType2, Metric2>::polyeval(const std::vector<double>& poly, const double z)
{
    const int n = poly.size();
    double sum = poly[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        sum *= z;
        sum += poly[i];
    }
    return sum;
}

template <class recType1, class Metric1, class recType2, class Metric2>
double MGC<recType1, Metric1, recType2, Metric2>::peak2ems(const std::vector<double>& data)
{
    double maxAbs = -1;
    double rms = 0;
    for (const auto v : data) {
        const double absV = abs(v);
        if (absV > maxAbs) {
            maxAbs = absV;
        }

        rms += v * v;
    }

    rms /= data.size();
    rms = sqrt(rms);

    return maxAbs / rms;
}

template <class recType1, class Metric1, class recType2, class Metric2>
std::vector<double> MGC<recType1, Metric1, recType2, Metric2>::linspace(double a, double b, int n)
{
    std::vector<double> array;
    if (n > 1) {
        double step = (b - a) / double(n - 1);
        int count = 0;
        while (count < n) {
            array.push_back(a + count * step);
            ++count;
        }
    } else {
        array.push_back(b);
    }

    return array;
}

template<class recType1, class Metric1, class recType2, class Metric2>
template<typename Container1, typename Container2>
std::vector<double>
MGC<recType1, Metric1, recType2, Metric2>::xcorr(const Container1 &a, const Container2 &b,
																const int n) const
{
	assert(a.size() == b.size());

	/* Compute distance matrices */
	auto X = computeDistanceMatrix<Container1>(a, metric1);
	auto Y = computeDistanceMatrix<Container2>(b, metric2);


	return MGC_direct().xcorr(X, Y, n);
}

}  // namespace metric
