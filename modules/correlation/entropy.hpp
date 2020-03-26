/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_DISTANCE_K_RANDOM_ENTROPY_HPP
#define _METRIC_DISTANCE_K_RANDOM_ENTROPY_HPP

#include <type_traits>

#include "../../3rdparty/blaze/Blaze.h"
#include "../distance/k-related/Standards.hpp"

//#include <vector>

namespace metric {


//namespace entropy {


template <typename recType, typename Metric = metric::Euclidian<typename recType::value_type>>
struct entropy { // averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing

    template <template <typename, typename> class OuterContainer, typename Container, typename OuterAllocator>
    double operator()(
            const OuterContainer<Container, OuterAllocator> & data,
            std::size_t k = 3,
            double logbase = 2,
            Metric metric = Metric(),
            bool exp = false
            ) const;

    template <template <typename, typename> class OuterContainer, template <typename, bool> class InnerContainer, class OuterAllocator, typename ValueType, bool F>
    double operator()( // TODO implement
            const OuterContainer<InnerContainer<ValueType, F>, OuterAllocator> & data, // inner cpntainer is specialized with bool F
            std::size_t k = 3,
            double logbase = 2,
            Metric metric = Metric(),
            bool exp = false
            ) const;

    template <typename Container>
    double estimate(
            const Container & a,
            const size_t sampleSize = 250,
            const double threshold = 0.05,
            size_t maxIterations = 1000,
            std::size_t k = 3,
            double logbase = 2,
            Metric metric = Metric(),
            bool exp = false
            );
};




template <typename recType, typename Metric = metric::Chebyshev<typename recType::value_type>>
struct entropy_kpN {

    template <template <typename, typename> class OuterContainer, typename Container, typename OuterAllocator>
    double operator()(
            const OuterContainer<Container, OuterAllocator> X,
            Metric metric = Metric(),
            size_t k = 7,
            size_t p = 70
            ) const;

    template <typename Container>
    double estimate(
            const Container & a,
            const size_t sampleSize = 250,
            const double threshold = 0.05,
            size_t maxIterations = 1000,
            std::size_t k = 3,
            double logbase = 2,
            Metric metric = Metric(),
            bool exp = false
            );
};



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


//} // namespace entropy

} // namespace metric

#include "entropy.cpp"
#endif
