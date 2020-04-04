/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#ifndef _METRIC_DISTANCE_K_RANDOM_VOI_CPP
#define _METRIC_DISTANCE_K_RANDOM_VOI_CPP

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <unordered_set>
#include <utility>
#include <vector>
#include <iterator>

#include <boost/functional/hash.hpp>

#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/digamma.hpp>

#include "../../space/tree.hpp"
#include "VOI.hpp"

namespace metric {

namespace {


    template <typename T>
    void print_vec(const std::vector<T>& v)
    {
        std::cout << "[";
        for (auto d : v) {
            std::cout << d << ", ";
        }
        std::cout << "]";
    }

    template <typename Node_ptr, typename Distance>
    void print(const std::vector<std::pair<Node_ptr, Distance>>& data)
    {
        std::cout << "[";
        for (auto& v : data) {
            print_vec(v.first->data);
            std::cout << " dist=" << v.second << "]";
        }
        std::cout << "]" << std::endl;
    }


    template <typename C1, typename C2, typename T=type_traits::underlaying_type_t<C1>>
    std::vector<std::vector<T>> combine(const C1& X, const C2& Y)
    {
        std::size_t N = X.size();
        std::size_t dx = X[0].size();
        std::size_t dy = Y[0].size();
        std::vector<std::vector<T>> XY(N);
        for (std::size_t i = 0; i < N; i++) {
            XY[i].resize(dx + dy);
            std::size_t k = 0;
            for (std::size_t j = 0; j < dx; j++, k++) {
                XY[i][k] = X[i][j];
            }
            for (std::size_t j = 0; j < dy; j++, k++) {
                XY[i][k] = Y[i][j];
            }
        }
        return XY;
    }


    template <typename Container, typename T = type_traits::underlaying_type_t<Container>>
    std::vector<T> unique(const Container& data)
    {
        std::unordered_set<std::size_t> hashes;
        std::vector<T> result;
        result.reserve(data.size());
        std::copy_if(data.begin(), data.end(), std::back_inserter(result),
            [&hashes](const T& i) { return hashes.insert(boost::hash_value(i)).second; });
        return result;
    }

}  // namespace


template <typename Container, typename Metric>
typename std::enable_if_t<!type_traits::is_container_of_integrals_v<Container>,
                          type_traits::underlaying_type_t<Container>>
mutualInformation(const Container & Xc, const Container & Yc, int k, const Metric & metric, int version)
{
    using T = type_traits::underlaying_type_t<Container>;
    using V = type_traits::index_value_type_t<Container>;
    T N = Xc.size();

    if (N < k + 1 || Yc.size() < k + 1)
        throw std::invalid_argument("number of points in dataset must be larger than k");

    std::vector<std::vector<T>> X;
    for (const auto& e: Xc)
        X.push_back(std::vector<T>(std::begin(e), std::end(e)));

    std::vector<std::vector<T>> Y;
    for (const auto& e: Yc)
        Y.push_back(std::vector<T>(std::begin(e), std::end(e)));

    std::vector<std::vector<T>> XY = combine(X, Y);
    metric::Tree<std::vector<T>, Metric> tree(XY, -1, metric);
    auto entropyEstimate = boost::math::digamma(k) + boost::math::digamma(N);
    if (version == 2) {
        entropyEstimate -= 1 / static_cast<double>(k);
    }

    metric::Tree<V, Metric> xTree(X, -1, metric);

    for (std::size_t i = 0; i < N; i++) {
        auto res = tree.knn(XY[i], k + 1);
        auto neighbor = res.back().first;
        auto dist = res.back().second;
        std::size_t nx = 0;
        if (version == 1) {
            auto dist_eps = std::nextafter(
                dist, std::numeric_limits<decltype(dist)>::max());  // this is instead of replacing < with <= in Tree //
                // added by Max F in order to match Julia code logic
                // without updating Tree
            nx = xTree.rnn(X[i], dist_eps).size();  // we include points that lay on the sphere
        } else if (version == 2) {
            auto ex = metric(X[neighbor->ID], X[i]);
            auto ex_eps = std::nextafter(
                ex, std::numeric_limits<decltype(ex)>::max());  // this it to include the most distant point into the
                // sphere // added by Max F in order to match Julia code
                // logic without updating Tree
            auto rnn_set = xTree.rnn(X[i], ex_eps);
            nx = rnn_set.size();  // replaced ex by ex_eps by Max F
        } else {
            throw std::runtime_error("this version is not allowed");
        }
        entropyEstimate -= 1.0 / N * boost::math::digamma(static_cast<double>(nx));
    }
    return entropyEstimate;
}


//template <typename Container, typename T>
//std::enable_if_t<type_traits::is_container_of_integrals_v<Container>, T>
//mutualInformation(const Container& Xc, const Container& Yc, T logbase)
//{
//    std::vector<std::vector<T>> XY = combine(Xc, Yc);
//    auto e = entropy<void>();
//    return e(Xc)
//        + e(Yc)  // entropy overload for integers is not implemented yet
//        - e(XY);
//}

template <typename C, typename Metric, typename T>
typename std::enable_if_t<!type_traits::is_container_of_integrals_v<C>, T>
variationOfInformation(const C& Xc, const C& Yc, int k, int p, T logbase)
{
    auto e = Entropy<void, Metric>(Metric(), k, p);
    //auto e = Entropy<void, metric::Chebyshev<T>>(metric::Chebyshev<T>(), k, p);
    //auto e = Entropy_simple<void, Metric>(Metric(), k);
    //auto e = Entropy_simple<void, metric::Chebyshev<T>>(metric::Chebyshev<T>(), k);
    auto result = e(Xc) + e(Yc) - 2 * mutualInformation<C>(Xc, Yc, k);
    if (result < 0)
        return 0;
    return result;
}

template <typename C, typename T>
typename std::enable_if_t<!type_traits::is_container_of_integrals_v<C>, T>
variationOfInformation_normalized(
        const C& Xc, const C& Yc, int k, int p, T logbase)
{
    using Cheb = metric::Chebyshev<T>;
    auto mi = mutualInformation<C>(Xc, Yc, k);
    auto e = Entropy<void, Cheb>(Cheb(), k, p);
    return 1 - (mi / (e(Xc) + e(Yc) - mi));
}

template <typename V>
template <typename C, typename El>
typename std::enable_if_t<!std::is_integral_v<El>, V>  // only real values are accepted
VOI<V>::operator()(const C& a, const C& b) const
{
    using Cheb = metric::Chebyshev<El>;
    auto e = Entropy<void, Cheb>(Cheb(), k, p);
    auto result = e(a) + e(b) - 2 * mutualInformation(a, b, k);
    if (result < 0)
        return 0;
    return result;
}

template <typename V>
template <typename C, typename El>
typename std::enable_if_t<std::is_integral_v<El>, V>
VOI<V>::operator()(const C& a, const C& b) const
{
    using Cheb = metric::Chebyshev<El>;

    auto e = Entropy<void, Cheb>(Cheb(), k, p);
    return e(a) + e(b)
        - 2 * mutualInformation(a, b, k);
}

template <typename V>
template <typename C, typename El>
typename std::enable_if_t<!std::is_integral_v<El>, V>  // only real values are accepted
VOI_normalized<V>::operator()(const C& a, const C& b) const
{
    using Cheb = metric::Chebyshev<El>;

    auto mi = mutualInformation(a, b, this->k);
    auto e = Entropy<void, Cheb>(Cheb(), this->k, this->p);
    return 1
        - (mi / (e(a) + e(b) - mi));
}



}  // namespace metric
#endif
