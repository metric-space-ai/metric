/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_DISTANCE_DETAILS_K_RELATED_ENTROPY_HPP
#define _METRIC_DISTANCE_DETAILS_K_RELATED_ENTROPY_HPP

#include <vector>
#include <type_traits>
#include "../../../distance/details/k-related/Standards.hpp"


namespace metric {
namespace distance {



template <typename T, typename Metric = metric::distance::Euclidian<T>>
typename std::enable_if<!std::is_integral<T>::value, T>::type
entropy(std::vector<std::vector<T>> data, std::size_t k = 3, T logbase = 2,
        Metric metric = metric::distance::Euclidian<T>());

// overload for integer types
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
entropy(std::vector<std::vector<T>> data, T logbase = 2);



template<typename T>
std::pair<std::vector<double>,std::vector<std::vector<T>>>
pluginEstimator(const std::vector<std::vector<T>> & Y);



template<typename T, typename Metric = metric::distance::Chebyshev<T>>
typename std::enable_if<!std::is_integral<T>::value, T>::type
mutualInformation(const std::vector<std::vector<T>> & Xc,
                  const std::vector<std::vector<T>> & Yc,
                  int k = 3,  Metric metric = Metric(), int version = 2);


template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
mutualInformation(const std::vector<std::vector<T>> & Xc,
                  const std::vector<std::vector<T>> & Yc, T logbase = 2.0);


template<typename T = double>
typename std::enable_if<!std::is_integral<T>::value, T>::type
variationOfInformation(const std::vector<std::vector<T>> & Xc,
                       const std::vector<std::vector<T>> & Yc, int k = 3, T logbase = 2.0);

template <typename T = double>
typename std::enable_if<!std::is_integral<T>::value, T>::type
variationOfInformation_normalized(const std::vector<std::vector<T>> &Xc,
                                  const std::vector<std::vector<T>> &Yc,
                                  int k = 3, T logbase = 2.0);


template <typename V = double>
struct VOI
{
    //using value_type = V; // on this stage we can not know actual value type of operands that will be passed
    using distance_type = V;

    int k = 3;
    V logbase = 2;

    explicit VOI() = default;
    explicit VOI(int k_ = 3, V logbase_ = 2): k(k_), logbase(logbase_) {}

    template<template<class, class> class Container, class Allocator_inner, class Allocator_outer, class El>
    typename std::enable_if<!std::is_integral<El>::value, V>::type  // only real values are accepted
    operator()(const Container<Container<El, Allocator_inner>, Allocator_outer> &a,
               const Container<Container<El, Allocator_inner>, Allocator_outer> &b) const;

    // TODO add support of 1D random values passed in simple containers

};



} // namespace distance
} // namespace metric

#include "entropy.cpp"
#endif
