/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_DISTANCE_K_RANDOM_VOI_HPP
#define _METRIC_DISTANCE_K_RANDOM_VOI_HPP

#include "../../../modules/distance/k-related/Standards.hpp"
#include "../../../modules/correlation/entropy.hpp"
#include "../../../modules/utils/type_traits.hpp"
#include <type_traits>
#include <vector>
namespace metric {


/**
 * @brief
 *
 * @param Xc
 * @param Yc
 * @param k
 * @param metric
 * @param version
 * @return
 */
template <typename Container, typename Metric = metric::Chebyshev<type_traits::underlaying_type_t<Container>>>
typename std::enable_if_t<!type_traits::is_container_of_integrals_v<Container>,
                          type_traits::underlaying_type_t<Container>>
mutualInformation(const Container& Xc,
    const Container & Yc, int k = 3, const Metric & metric = Metric(), int logbase = 2);

template <typename Container, typename T = type_traits::underlaying_type_t<Container> >
std::enable_if_t<type_traits::is_container_of_integrals_v<Container>, T>
mutualInformation(const Container& Xc, const Container& Yc, T logbase = 2.0);

/**
 * @brief
 *
 * @param Xc
 * @param Yc
 * @param k
 * @param logbase
 * @return
 */
template <typename C, typename Metric = metric::Chebyshev<type_traits::underlaying_type_t<C>>,
          typename T = type_traits::underlaying_type_t<C>>
typename std::enable_if_t<!type_traits::is_container_of_integrals_v<C>, T>
variationOfInformation(const C& Xc, const C& Yc, int k = 3, int p = 25, T logbase = 2.0);


/**
 * @brief
 *
 * @param Xc
 * @param Yc
 * @param k
 * @param logbase
 * @return
 */
    template <typename C, typename T = type_traits::underlaying_type_t<C>>
    typename std::enable_if_t<!type_traits::is_container_of_integrals_v<C>,T>
    variationOfInformation_normalized(
    const C & Xc, const C& Yc, int k = 3, int p = 25, T logbase = 2.0);

/**
 * @brief
 *
 * @tparam V
 */
template <typename V = double>
struct VOI {
    using distance_type = V;

    int k = 3;
    int p = 25;
    V logbase = 2;

    /**
     * @brief Construct a new VOI object
     *
     * @param k_
     * @param logbase_
     */
    explicit VOI(int k_ = 3, V logbase_ = 2, int p_ = 25)
        : k(k_)
        , p(p_)
        , logbase(logbase_)
    {
    }

    /**
     * @brief calculate variation of information between two containers
     *
     * @param a first container
     * @param b second container
     * @return variation of information between a and b
     */

    template <typename Container, typename T = type_traits::underlaying_type_t<Container>>
    typename std::enable_if_t<!std::is_integral_v<T>, V>  // only real values are accepted
    operator()(const Container& a, const Container& b) const;

    template <typename Container, typename T = type_traits::underlaying_type_t<Container>>
    typename std::enable_if_t<std::is_integral_v<T>, V>  // only real values are accepted
    operator()(const Container& a, const Container& b) const;

    // TODO add support of 1D random values passed in simple containers
};
// deduction guide for VOI
template <typename V>
VOI(int, V)->VOI<double>;

/**
 * @class VOI_normalized 
 * 
 * @brief Normalized Variation of Information
 *
 */
template <typename V = double>
struct VOI_normalized : VOI<V> {
    /**
     * @brief Construct a new VOI_normalized object
     *
     * @param k_
     * @param logbase_
     */
    explicit VOI_normalized(int k_ = 3, V logbase_ = 2)
        : VOI<V>(k_, logbase_)
    {
    }

    /**
     * @brief Calculate Variation of Information 
     *
     * @param a first container
     * @param b second container
     * @return varition of information between a and b
     */
    template <typename Container, typename T = type_traits::underlaying_type_t<Container>>
    typename std::enable_if_t<!std::is_integral_v<T>, V>  // only real values are accepted
    operator()(const Container& a, const Container& b) const;

    // TODO add support of 1D random values passed in simple containers
};

template <typename V>
VOI_normalized(int, V)->VOI_normalized<double>;



}  // namespace metric

#include "VOI.cpp"
#endif
