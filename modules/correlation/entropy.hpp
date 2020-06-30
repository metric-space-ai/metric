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
#include "../modules/utils/type_traits.hpp"

namespace metric {


template <typename RecType, typename Metric = metric::Euclidean<typename RecType::value_type>>
class EntropySimple { // averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing
public:
    EntropySimple(Metric metric = Metric(), size_t k = 3, bool exp = false) :
        metric(metric),
        k(k),
        exp(exp),
        logbase(2) {} // TODO remove (?)

    template <typename Container>
    double operator()(const Container& data) const;
    //double operator()(const Container& data, bool avoid_repeated = false) const;

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



// https://hal.inria.fr/hal-01272527/document
template <typename RecType, typename Metric = metric::Chebyshev<typename RecType::value_type>>
class Entropy {
public:
    Entropy(Metric metric = Metric(), size_t k = 7, size_t p = 70, bool exp = false) :
        metric(metric),
        k(k),
        p(p),
        exp(exp) {}

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


// VOI & VMixing


/**
 * @brief
 *
 * @param Xc
 * @param Yc
 * @param k
 * @param logbase
 * @return
 */
template <typename C, typename Metric = metric::Chebyshev<type_traits::underlying_type_t<C>>>
typename std::enable_if_t<!type_traits::is_container_of_integrals_v<C>, type_traits::underlying_type_t<C>>
variationOfInformationSimple(const C& Xc, const C& Yc, int k = 3);


/**
 * @brief
 *
 * @param Xc
 * @param Yc
 * @param k
 * @param logbase
 * @return
 */
template <typename C, typename Metric = metric::Chebyshev<type_traits::underlying_type_t<C>>>
typename std::enable_if_t<!type_traits::is_container_of_integrals_v<C>, type_traits::underlying_type_t<C>>
variationMixedSimple(const C& Xc, const C& Yc, int k = 3);



/**
 * @brief
 *
 * @param Xc
 * @param Yc
 * @param k
 * @param logbase
 * @return
 */
template <typename C, typename Metric = metric::Chebyshev<type_traits::underlying_type_t<C>>>
typename std::enable_if_t<!type_traits::is_container_of_integrals_v<C>, type_traits::underlying_type_t<C>>
variationOfInformation_kpN(const C& Xc, const C& Yc, int k = 3, int p = 25);



/**
 * @brief
 *
 * @param Xc
 * @param Yc
 * @param k
 * @param logbase
 * @return
 */
template <typename C, typename Metric = metric::Chebyshev<type_traits::underlying_type_t<C>>>
typename std::enable_if_t<!type_traits::is_container_of_integrals_v<C>, type_traits::underlying_type_t<C>>
variationMixed_kpN(const C& Xc, const C& Yc, int k = 3, int p = 25);






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
