/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_DISTANCE_K_RELATED_L1_HPP
#define _METRIC_DISTANCE_K_RELATED_L1_HPP

#include "../../../3rdparty/blaze/Blaze.h"

namespace metric {
/**
 * @brief
 *
 * @param a
 * @param b
 * @return
 */
template <typename T>
double sorensen(const T& a, const T& b);

/**
 * @brief
 *
 * @param a
 * @param b
 * @return
 */
template <typename Value>
double sorensen(const blaze::CompressedVector<Value>& a, const blaze::CompressedVector<Value>& b);

template <typename V = double>
struct Hassanat {
    using value_type = V;
    using distance_type = value_type;

    explicit Hassanat() = default;

    /**
     * @brief Calculate Hassanat distance in R^n
     *
     * @param a first vector
     * @param b second vector
     * @return Hassanat distance between a and b
     */

    template <typename Container>
    distance_type operator()(const Container& a, const Container& b) const;
};

template <typename Value>
double sorensen(const blaze::CompressedVector<Value>& a, const blaze::CompressedVector<Value>& b);

template <typename V = double>
struct Ruzicka {
    using value_type = V;
    using distance_type = value_type;

    explicit Ruzicka() = default;

    /**
     * @brief Calculate Ruzicka distance in R^n
     *
     * @param a first vector
     * @param b second vector
     * @return Ruzicka distance between a and b
     */

    template <typename Container>
    distance_type operator()(const Container& a, const Container& b) const;
};



}  // namespace metric

#include "L1.cpp"

#endif  // Header Guard
