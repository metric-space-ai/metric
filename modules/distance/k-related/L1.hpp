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

template <typename V = double>
class Sorensen {
public:
    using distance_type = V;

    explicit Sorensen() = default;

    /**
     * @brief
     *
     * @param a
     * @param b
     * @return
     */
    template <typename Container>
    typename std::enable_if<!std::is_same<Container, V>::value, distance_type>::type
    operator()(const Container& a, const Container& b) const;

    /**
     * @brief
     *
     * @param a
     * @param b
     * @return
     */
    distance_type operator()(const blaze::CompressedVector<V>& a, const blaze::CompressedVector<V>& b) const;

    // TODO add support of 1D random values passed in simple containers
};

}  // namespace metric

#include "L1.cpp"

#endif  // Header Guard
