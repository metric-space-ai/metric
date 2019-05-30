// Copyright (c) 2019 Michael Welsch


#ifndef METRIC_WAVLET_H
#define METRIC_WAVLET_H

#include <iostream>
//#include <stdexcept>
#include <tuple>
#include <cmath>
#include <vector>
//#include <numeric>
#include <algorithm>
#include <functional>
//#include <deque>
#include "../../../3rdparty/blaze/Math.h"


namespace wavelet {

// valid convolution
template <typename T>
std::vector<T>
conv_valid(std::vector<T> const &f, std::vector<T> const &g);

// full convolution
template <typename T>
std::vector<T>
conv(std::vector<T> const &f, std::vector<T> const &g);

// linspace (erzeugt einen linearen Datenvektor)
template<typename T>
std::vector<T>
linspace(T a, T b, int n);

// upsconv
template <typename T>
std::vector<T>
upsconv(std::vector<T> const &x, std::vector<T> const &f, int len);

// dbwavf
template<typename T>
std::vector<T>
dbwavf(int const wnum, T dings);

// orthfilt
template<typename T>
std::tuple<std::vector<T>, std::vector<T>, std::vector<T>, std::vector<T>>
orthfilt(std::vector<T> const &W_in);

// dwt
template<typename T>
std::tuple<std::vector<T>, std::vector<T>>
dwt(std::vector<T> const &x, int waveletType);

// idwt
template<typename T>
std::vector<T>
idwt(std::vector<T> a, std::vector<T> d, int waveletType, int lx);

// wmaxlev
int 
wmaxlev(int sizeX, int waveletType);

// wavedec;
template<typename T>
std::deque<std::vector<T>>
wavedec(std::vector<T> const &x, int order, int waveletType);

// waverec
template<typename T>
std::vector<T>
waverec(std::deque<std::vector<T>> const &subBands, int waveletType);

//denoise
template<typename T>
std::deque<std::vector<T>>
denoise(std::deque<std::vector<T>> const &data, T const &tresh);

// sparse
template<typename T>
std::tuple<std::deque<std::vector<T>>, std::deque<std::vector<T>>>
sparse(std::deque<std::vector<T>> const &data);

// distance measure by time elastic cost matrix.
template <typename T>
T TWED(blaze::CompressedVector<T> const &As, blaze::CompressedVector<T> const &Bs, T const &penalty = 0, T const &elastic = 1);



} // namespace

#include "wavelet.cpp"

#endif // METRIC_WAVLET_H