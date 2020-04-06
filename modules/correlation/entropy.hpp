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


namespace metric {


template <typename RecType, typename Metric = metric::Euclidean<typename RecType::value_type>>
struct Entropy_simple { // averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing

    Entropy_simple(Metric metric_ = Metric(), size_t k_ = 3, bool exp_ = false) :
        metric(metric_),
        k(k_),
        exp(exp_),
        logbase(2) {} // TODO remove (?)

    template <typename Container>
    double operator()(const Container& data) const;

    template <template <typename, typename> class OuterContainer, template <typename, bool> class InnerContainer, class OuterAllocator, typename ValueType, bool F>
    double operator()( // TODO implement
            const OuterContainer<InnerContainer<ValueType, F>, OuterAllocator> & data // inner cpntainer is specialized with bool F
    ) const;

    template <typename Container>
    double estimate(
            const Container & a,
            const size_t sampleSize = 250,
            const double threshold = 0.05,
            size_t maxIterations = 1000
    ) const;

private:
    size_t k;
    size_t p;
    Metric metric;
    double logbase;
    bool exp;
};




template <typename RecType, typename Metric = metric::Chebyshev<typename RecType::value_type>>
struct Entropy {

    Entropy(Metric metric_ = Metric(), size_t k_ = 7, size_t p_ = 70, bool exp_ = false) :
        metric(metric_),
        k(k_),
        p(p_),
        exp(exp_) {}

    template <typename Container>
    double operator()(const Container& data) const;

    template <typename Container>
    double estimate(
            const Container & a,
            const size_t sampleSize = 250,
            const double threshold = 0.05,
            size_t maxIterations = 1000
    ) const;

private:
    size_t k;
    size_t p;
    Metric metric;
    bool exp;
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


} // namespace metric

#include "entropy.cpp"
#endif
