/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#ifndef _METRIC_DISTANCE_K_RANDOM_ENTROPY_HPP
#define _METRIC_DISTANCE_K_RANDOM_ENTROPY_HPP

#include "../../3rdparty/blaze/Blaze.h"
#include "../distance/k-related/Standards.hpp"

#include <type_traits>
#include <functional>


namespace metric {

// code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing
template <typename Container>
double estimate(
    const Container& a,
    const std::function<double(const Container&)>& entropy,
    const size_t sampleSize = 250,
    const double threshold = 0.05,
    size_t maxIterations = 1000
);

// averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing
template <typename Container, typename Metric>
double entropy(
    const Container& data,
    std::size_t k = 3,
    double logbase = 2,
    Metric metric = Metric(),
    bool exp = false
);


template <typename Container, typename Metric>
double entropy_kpN(
    const Container& data,
    size_t k = 7,
    size_t p = 70,
    Metric metric = Metric()
);


// -------------------------------- to be debugged

// ported from Julia, not in use
/**
 * @brief
 *
 * @param Y
 * @return
 */
template <typename T>
std::pair<std::vector<double>, std::vector<std::vector<T>>> pluginEstimator(const std::vector<std::vector<T>>& Y);


} // namespace metric

#include "entropy.cpp"
#endif
