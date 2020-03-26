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

}  // namespace metric

#include "L1.cpp"

#endif  // Header Guard
