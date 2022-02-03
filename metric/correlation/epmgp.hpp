/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

// approximation of probability for multidimensional normal distribution bounded by (hyper)rectangle
// https://arxiv.org/pdf/1111.6832.pdf
// based on local_gaussian.m Matlab code

#ifndef _EPMGP_HPP
#define _EPMGP_HPP

#include <blaze/Blaze.h>

#include <cmath>
#include <tuple>
#include <vector>

namespace epmgp {

template <typename T> int sgn(T val)
{ // sign for arbitrary type
	return (T(0) < val) - (val < T(0));
}

template <typename T> T erfcx_simple(T x)
{ // for double on x86_64, inf starts at -26
	return std::exp(x * x) * std::erfc(x);
}

// https://stackoverflow.com/questions/39777360/accurate-computation-of-scaled-complementary-error-function-erfcx
double erfcx_double(double x);

template <typename T> T erfcx(T x)
{ // for double, inf starts at -26 on x86_64
	return (T)erfcx_double((double)x);
}

template <typename T>
auto truncNormMoments(std::vector<T> lowerBIN, std::vector<T> upperBIN, std::vector<T> muIN, std::vector<T> sigmaIN)
	-> std::tuple<std::vector<T>, std::vector<T>, std::vector<T>>;

template <typename T>
auto local_gaussian_axis_aligned_hyperrectangles(blaze::DynamicVector<T> m, blaze::DynamicMatrix<T> K,
												 blaze::DynamicVector<T> lowerB, blaze::DynamicVector<T> upperB)
	-> std::tuple<T, blaze::DynamicVector<T>, blaze::DynamicMatrix<T>>;

} // namespace epmgp

#include "epmgp.cpp"

#endif // _EPMGP_HPP
