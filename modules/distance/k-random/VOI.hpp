/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_DISTANCE_K_RANDOM_VOI_HPP
#define _METRIC_DISTANCE_K_RANDOM_VOI_HPP

#include "../k-related/Standards.hpp"
#include <type_traits>
#include <vector>

namespace metric {

///**
// * @brief Continuous entropy estimator
// *
// * @param data
// * @param k
// * @param logbase
// * @param metric
// * @param exp  - flag, applies 1/exp(1)*exp(estimated_entropy) to output
// * @return value of entropy estimation of the data
// */
//template <typename Container, typename Metric = metric::Euclidian<typename Container::value_type>, typename L = double>
//double entropy_fn(  // old erronuous version, TODO remove
//    std::vector<Container> data, std::size_t k = 3, L logbase = 2, Metric metric = Metric(), bool exp = false);


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


/**
 * @brief
 *
 * @param Y
 * @return
 */
template <typename T>
std::pair<std::vector<double>, std::vector<std::vector<T>>> pluginEstimator(const std::vector<std::vector<T>>& Y);

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
template <typename T, typename Metric = metric::Chebyshev<T>>
typename std::enable_if<!std::is_integral<T>::value, T>::type mutualInformation(const std::vector<std::vector<T>>& Xc,
    const std::vector<std::vector<T>>& Yc, int k = 3, Metric metric = Metric(), int version = 2);

template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type mutualInformation(
    const std::vector<std::vector<T>>& Xc, const std::vector<std::vector<T>>& Yc, T logbase = 2.0);

/**
 * @brief
 *
 * @param Xc
 * @param Yc
 * @param k
 * @param logbase
 * @return
 */
template <typename T = double, typename Metric = metric::Chebyshev<T>>
typename std::enable_if<!std::is_integral<T>::value, T>::type variationOfInformation(
    const std::vector<std::vector<T>>& Xc, const std::vector<std::vector<T>>& Yc, int k = 3, T logbase = 2.0);

/**
 * @brief
 *
 * @param Xc
 * @param Yc
 * @param k
 * @param logbase
 * @return
 */
template <typename T = double>
typename std::enable_if<!std::is_integral<T>::value, T>::type variationOfInformation_normalized(
    const std::vector<std::vector<T>>& Xc, const std::vector<std::vector<T>>& Yc, int k = 3, T logbase = 2.0);

/**
 * @brief
 *
 * @tparam V
 */
template <typename V = double>
struct VOI {
    using distance_type = V;

    int k = 3;
    V logbase = 2;

    /**
     * @brief Construct a new VOI object
     *
     * @param k_
     * @param logbase_
     */
    explicit VOI(int k_ = 3, V logbase_ = 2)
        : k(k_)
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
    template <template <class, class> class Container, class Allocator_inner, class Allocator_outer, class El>
    typename std::enable_if<!std::is_integral<El>::value, V>::type  // only real values are accepted
    operator()(const Container<Container<El, Allocator_inner>, Allocator_outer>& a,
        const Container<Container<El, Allocator_inner>, Allocator_outer>& b) const;

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
    template <template <class, class> class Container, class Allocator_inner, class Allocator_outer, class El>
    typename std::enable_if<!std::is_integral<El>::value, V>::type  // only real values are accepted
    operator()(const Container<Container<El, Allocator_inner>, Allocator_outer>& a,
        const Container<Container<El, Allocator_inner>, Allocator_outer>& b) const;

    // TODO add support of 1D random values passed in simple containers
};

template <typename V>
VOI_normalized(int, V)->VOI_normalized<double>;



// functions based on Kozachenko-Leonenko entropy estimator
// WARNING the entropy_kl estimator function used in functions declared below, is not finished and seems to have BUGS


/**
 * @class VOI_kl
 * 
 * @brief VOI based on Kozachenko-Leonenko entropy estimator
 *
 */
template <typename V = double>
struct VOI_kl {
    using distance_type = V;

    int k = 3;
    V logbase = 2;

    /**
     * @brief Construct a new VOI_kl object
     *
     * @param k_
     * @param logbase_
     */
    explicit VOI_kl(int k_ = 3, V logbase_ = 2)
        : k(k_)
        , logbase(logbase_)
    {
    }

    /**
     * @brief Calculate variation of informatin based on Kozachenko-Leonenko entropy estimator
     *
     * @param a first container
     * @param b second container
     * @return variation of information
     */
    template <template <class, class> class Container, class Allocator_inner, class Allocator_outer, class El>
    typename std::enable_if<!std::is_integral<El>::value, V>::type  // only real values are accepted
    operator()(const Container<Container<El, Allocator_inner>, Allocator_outer>& a,
        const Container<Container<El, Allocator_inner>, Allocator_outer>& b) const;

    // TODO add support of 1D random values passed in simple containers
};

template <typename V>
VOI_kl(int, V)->VOI_kl<double>;

/**
 * @class VOI_normalized_kl
 * @brief Normalized Variation of Information based on Kozachenko-Leonenko entropy estimator
 *
 */
template <typename V = double>
struct VOI_normalized_kl : VOI_kl<V> {
    /**
     * @brief Construct a new VOI_normalized_kl object
     *
     * @param k_
     * @param logbase_
     */
    explicit VOI_normalized_kl(int k_ = 3, V logbase_ = 2)
        : VOI_kl<V>(k_, logbase_)
    {
    }

    /**
     * @brief calculate normalized value of variation of information
     *
     * @param a first value
     * @param b second value
     * @return normalized value of the variation of information
     */
    template <template <class, class> class Container, class Allocator_inner, class Allocator_outer, class El>
    typename std::enable_if<!std::is_integral<El>::value, V>::type  // only real values are accepted
    operator()(const Container<Container<El, Allocator_inner>, Allocator_outer>& a,
        const Container<Container<El, Allocator_inner>, Allocator_outer>& b) const;

    // TODO add support of 1D random values passed in simple containers
};

// deduction guide for VOI_normalized_kl
template <typename V>
VOI_normalized_kl(int, V)->VOI_normalized_kl<double>;





// helper functions for averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing


std::vector<double> linspace(double a, double b, int n);

double polyeval(const std::vector<double>& poly, const double z);

double erfcinv(const double z);

double erfinv_imp(const double p, const double q);

std::vector<double> icdf(const std::vector<double>& prob, const double mu, const double sigma);

double variance(const std::vector<double>& data, const double mean);

double mean(const std::vector<double>& data);

double peak2ems(const std::vector<double>& data);



}  // namespace metric

#include "VOI.cpp"
#endif
