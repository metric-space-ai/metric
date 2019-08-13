/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Michael Welsch
*/

#ifndef _METRIC_TRANSFORM_WAVLET_HPP
#define _METRIC_TRANSOFRM_WAVLET_HPP

#include <iostream>
#include <tuple>
#include <cmath>
#include <vector>
#include <algorithm>
#include <functional>
#include <deque>
#include "../../3rdparty/blaze/Math.h"

namespace wavelet {

/**
 * @brief valid convolution
 * 
 * @param f 
 * @param g 
 * @return 
 */
template <typename T>
std::vector<T> conv_valid(std::vector<T> const& f, std::vector<T> const& g);

/**
 * @brief full convolution
 * 
 * @param f 
 * @param g 
 * @return 
 */
template <typename T>
std::vector<T> conv(std::vector<T> const& f, std::vector<T> const& g);

/**
 * @brief linspace (erzeugt einen linearen Datenvektor)
 * 
 * @param a 
 * @param b 
 * @param n 
 * @return
 */
template <typename T>
std::vector<T> linspace(T a, T b, int n);

/**
 * @brief upsconv
 * 
 * @param x 
 * @param f 
 * @param len 
 * @return
 */
template <typename T>
std::vector<T> upsconv(std::vector<T> const& x, std::vector<T> const& f, int len);

/**
 * @brief 
 * 
 * @param wnum 
 * @param dings 
 * @return
 */
template <typename T>
std::vector<T> dbwavf(int const wnum, T dings);

/**
 * @brief 
 * 
 * @param W_in 
 * @return
 */
template <typename T>
std::tuple<std::vector<T>, std::vector<T>, std::vector<T>, std::vector<T>> orthfilt(std::vector<T> const& W_in);

/**
 * @brief 
 * 
 * @param x 
 * @param waveletType 
 * @return
 */
template <typename T>
std::tuple<std::vector<T>, std::vector<T>> dwt(std::vector<T> const& x, int waveletType);

/**
 * @brief 
 * 
 * @param a 
 * @param d 
 * @param waveletType 
 * @param lx 
 * @return
 */
template <typename T>
std::vector<T> idwt(std::vector<T> a, std::vector<T> d, int waveletType, int lx);

/**
 * @brief 
 * 
 * @param sizeX 
 * @param waveletType 
 * @return
 */
int wmaxlev(int sizeX, int waveletType);

/**
 * @brief 
 * 
 * @param x 
 * @param order 
 * @param waveletType 
 * @return
 */
template <typename T>
std::deque<std::vector<T>> wavedec(std::vector<T> const& x, int order, int waveletType);

/**
 * @brief 
 * 
 * @param subBands 
 * @param waveletType 
 * @return
 */
template <typename T>
std::vector<T> waverec(std::deque<std::vector<T>> const& subBands, int waveletType);

/**
 * @brief 
 * 
 * @param data 
 * @param tresh 
 * @return
 */
template <typename T>
std::deque<std::vector<T>> denoise(std::deque<std::vector<T>> const& data, T const& tresh);

/**
 * @brief 
 * 
 * @param data 
 * @return
 */
template <typename T>
std::tuple<std::deque<std::vector<T>>, std::deque<std::vector<T>>> sparse(std::deque<std::vector<T>> const& data);

/**
 * @brief distance measure by time elastic cost matrix.
 * 
 * @param As 
 * @param Bs 
 * @param penalty 
 * @param elastic 
 * @return 
 */
template <typename T>
T TWED(blaze::CompressedVector<T> const& As, blaze::CompressedVector<T> const& Bs, T const& penalty = 0,
    T const& elastic = 1);

}  // namespace

#include "wavelet.cpp"

#endif