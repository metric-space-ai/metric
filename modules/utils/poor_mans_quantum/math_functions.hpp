/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Michael Welsch
*/

#ifndef _METRIC_UTILS_POOR_MANS_QUANTUM_MATH_FUNCTIONS_HPP
#define _METRIC_UTILS_POOR_MANS_QUANTUM_MATH_FUNCTIONS_HPP

#include <vector>

namespace metric {

/**
 * @brief 
 * 
 * @param a 
 * @param b 
 * @param n 
 * @return
 */
template <typename T>
std::vector<T> linspace(T a, T b, int n);


/**
 * @brief akima interpolation
 * 
 * @details  Ref. : Hiroshi Akima, Journal of the ACM, Vol. 17, No. 4, October 1970, pages 589-602.
 * @param x 
 * @param y 
 * @param xi 
 * @param save_Mode 
 * @return
 */
template <typename T>
std::vector<T> akimaInterp1(
    std::vector<T> const& x, std::vector<T> const& y, std::vector<T> const& xi, bool save_Mode = true);

/**
 * @brief 
 * 
 * @param y 
 * @param n 
 * @return
 */
template <typename T>
std::vector<T> resize(std::vector<T> y, size_t n);

/**
 * @brief  Transpose matrix
 * 
 * @param a input matrix
 * @return result of transposition of matrix a
 */
template <typename T>
std::vector<std::vector<T>> transpose(std::vector<std::vector<T>>& a);

/**
 * @brief 
 * 
 * @param data 
 * @param probs 
 * @return
 */
template <typename T>
T quickQuantil(std::vector<T> data, T probs);

}  // end namespace

#include "math_functions.cpp"
#endif  //header guard
