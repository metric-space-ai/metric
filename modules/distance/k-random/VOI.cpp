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
#include <vector>

#include <boost/functional/hash.hpp>

#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/digamma.hpp>
//#include <boost/math/special_functions/gamma.hpp>

#include "../../space/tree.hpp"
#include "estimator_helpers.cpp"
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


    template <typename T>
    void combine(
        const std::vector<std::vector<T>>& X, const std::vector<std::vector<T>>& Y, std::vector<std::vector<T>>& XY)
    {
        std::size_t N = X.size();
        std::size_t dx = X[0].size();
        std::size_t dy = Y[0].size();
        XY.resize(N);
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
    }
    template <typename T>
    std::vector<T> unique(const std::vector<T>& data)
    {
        std::unordered_set<std::size_t> hashes;
        std::vector<T> result;
        result.reserve(data.size());
        std::copy_if(data.begin(), data.end(), std::back_inserter(result),
            [&hashes](const T& i) { return hashes.insert(boost::hash_value(i)).second; });
        return result;
    }

}  // namespace




template <typename T, typename Metric>
typename std::enable_if<!std::is_integral<T>::value, T>::type mutualInformation(
    const std::vector<std::vector<T>>& Xc, const std::vector<std::vector<T>>& Yc, int k, Metric metric, int version)
{
    T N = Xc.size();

    if (N < k + 1 || Yc.size() < k + 1)
        throw std::invalid_argument("number of points in dataset must be larger than k");

    auto X = Xc;
    auto Y = Yc;
    //add_noise(X);
    //add_noise(Y);
    std::vector<std::vector<T>> XY;
    combine(X, Y, XY);
    metric::Tree<std::vector<T>, Metric> tree(XY, -1, metric);
    auto entropyEstimate = boost::math::digamma(k) + boost::math::digamma(N);
    if (version == 2) {
        entropyEstimate -= 1 / static_cast<double>(k);
    }

    metric::Tree<std::vector<T>, Metric> xTree(X, -1, metric);

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

template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type mutualInformation(
    //const std::vector<std::vector<T>>& Xc, const std::vector<std::vector<T>>& Yc, T logbase)
    const std::vector<std::vector<T>>& Xc, const std::vector<std::vector<T>>& Yc, T k)
{
    std::vector<std::vector<T>> XY;
    combine(Xc, Yc, XY);
//    return entropy<T>(Xc, logbase)
//        + entropy<T>(Yc,
//            logbase)  // entropy overload for integers is not implemented yet
//        - entropy<T>(XY, logbase);
//    return metric::conv_diff_entropy_inv(entropy<T>(Xc, logbase))
//        + metric::conv_diff_entropy_inv(entropy<T>(Yc,
//            logbase))  // entropy overload for integers is not implemented yet
//        - metric::conv_diff_entropy_inv(entropy<T>(XY, logbase));

//    return entropy<T>(Xc, k, 2.0, metric::Euclidian<T>(), false) // TODO update metric and logbase
//        + entropy<T>(Yc, k, 2.0, metric::Euclidian<T>(), false)  // entropy overload for integers is not implemented yet
//        - entropy<T>(XY, k, 2.0, metric::Euclidian<T>(), false);

    auto e = entropy<void, metric::Euclidian<T>>(); // TODO update metric and logbase
    return e(Xc, k, 2.0)
        + e(Yc, k, 2.0)  // entropy overload for integers is not implemented yet
        - e(XY, k, 2.0);

}

template <typename T, typename Metric>
typename std::enable_if<!std::is_integral<T>::value, T>::type variationOfInformation(
    const std::vector<std::vector<T>>& Xc, const std::vector<std::vector<T>>& Yc, int k, T logbase)
{
//    return entropy<std::vector<T>, Metric>(Xc, k, logbase, Metric()) + entropy<std::vector<T>, Metric>(Yc, k, logbase, Metric())
//        - 2 * mutualInformation<T>(Xc, Yc, k);
//    return metric::conv_diff_entropy_inv(entropy<std::vector<T>, Metric>(Xc, k, logbase, Metric())) + metric::conv_diff_entropy_inv(entropy<std::vector<T>, Metric>(Yc, k, logbase, Metric()))
//        - 2 * mutualInformation<T>(Xc, Yc, k);

//    return entropy_fn<std::vector<T>, Metric>(Xc, k, logbase, Metric(), false) + entropy_fn<std::vector<T>, Metric>(Yc, k, logbase, Metric(), false)
//        - 2 * mutualInformation<T>(Xc, Yc, k);

    auto e = entropy<void, Metric>();
    return e(Xc, k, logbase) + e(Yc, k, logbase) // TODD probably metric metaparameters needed!
        - 2 * mutualInformation<T>(Xc, Yc, k);

}

template <typename T>
typename std::enable_if<!std::is_integral<T>::value, T>::type variationOfInformation_normalized(
    const std::vector<std::vector<T>>& Xc, const std::vector<std::vector<T>>& Yc, int k, T logbase)
{
    using Cheb = metric::Chebyshev<T>;
    auto mi = mutualInformation<T>(Xc, Yc, k);
    //return 1 - (mi / (entropy<std::vector<T>, Cheb>(Xc, k, logbase, Cheb()) + entropy<std::vector<T>, Cheb>(Yc, k, logbase, Cheb()) - mi));
    //return 1 - (mi / (metric::conv_diff_entropy_inv(entropy<std::vector<T>, Cheb>(Xc, k, logbase, Cheb())) + metric::conv_diff_entropy_inv(entropy<std::vector<T>, Cheb>(Yc, k, logbase, Cheb()) - mi)));

    //return 1 - (mi / (entropy_fn<std::vector<T>, Cheb>(Xc, k, logbase, Cheb(), false) + entropy_fn<std::vector<T>, Cheb>(Yc, k, logbase, Cheb(), false) - mi));
    auto e = entropy<void, Cheb>();
    return 1 - (mi / (e(Xc, k, logbase) + e(Yc, k, logbase) - mi));
}

template <typename V>
template <template <class, class> class Container, class Allocator_inner, class Allocator_outer, class El>
typename std::enable_if<!std::is_integral<El>::value, V>::type VOI<V>::operator()(
    const Container<Container<El, Allocator_inner>, Allocator_outer>& a,
    const Container<Container<El, Allocator_inner>, Allocator_outer>& b) const
{
    using Cheb = metric::Chebyshev<El>;
//    return entropy_fn<std::vector<El>, Cheb>(a, k, logbase, Cheb()) + entropy_fn<std::vector<El>, Cheb>(b, k, logbase, Cheb())
//        - 2 * mutualInformation<El>(a, b, k);
//    return metric::conv_diff_entropy_inv(entropy_fn<std::vector<El>, Cheb>(a, k, logbase, Cheb())) + metric::conv_diff_entropy_inv(entropy_fn<std::vector<El>, Cheb>(b, k, logbase, Cheb()))
//        - 2 * mutualInformation<El>(a, b, k);

//    return entropy_fn<std::vector<El>, Cheb>(a, k, logbase, Cheb(), false) + entropy_fn<std::vector<El>, Cheb>(b, k, logbase, Cheb(), false)
//        - 2 * mutualInformation<El>(a, b, k);

    auto e = entropy<void, Cheb>();
    return e(a, k, logbase) + e(b, k, logbase)
        - 2 * mutualInformation<El>(a, b, k);

}

template <typename V>
template <template <class, class> class Container, class Allocator_inner, class Allocator_outer, class El>
typename std::enable_if<!std::is_integral<El>::value, V>::type VOI_normalized<V>::operator()(
    const Container<Container<El, Allocator_inner>, Allocator_outer>& a,
    const Container<Container<El, Allocator_inner>, Allocator_outer>& b) const
{
    using Cheb = metric::Chebyshev<El>;
    auto mi = mutualInformation<El>(a, b, this->k);
//    return 1
//        - (mi
//            / (entropy_fn<std::vector<El>, Cheb>(a, this->k, this->logbase, Cheb())
//                + entropy_fn<std::vector<El>, Cheb>(b, this->k, this->logbase, Cheb()) - mi));
//    return 1
//        - (mi
//            / (metric::conv_diff_entropy_inv(entropy<std::vector<El>, Cheb>(a, this->k, this->logbase, Cheb()))
//                + metric::conv_diff_entropy_inv(entropy<std::vector<El>, Cheb>(b, this->k, this->logbase, Cheb()) - mi)));

//    return 1
//        - (mi
//            / (entropy_fn<std::vector<El>, Cheb>(a, this->k, this->logbase, Cheb(), false)
//                + entropy_fn<std::vector<El>, Cheb>(b, this->k, this->logbase, Cheb(), false) - mi));

    auto e = entropy<void, Cheb>();
    return 1
        - (mi
            / (e(a, this->k, this->logbase)
                + e(b, this->k, this->logbase) - mi));
}

// VOI based on Kozachenko-Leonenko entropy estimator
// WARNING: the entropy_kl function called below, seems to have bugs!
// TODO debug entropy_kl, add support of arbitrary metric

template <typename V>
template <template <class, class> class Container, class Allocator_inner, class Allocator_outer, class El>
typename std::enable_if<!std::is_integral<El>::value, V>::type VOI_kl<V>::operator()(
    const Container<Container<El, Allocator_inner>, Allocator_outer>& a,
    const Container<Container<El, Allocator_inner>, Allocator_outer>& b) const
{
    std::vector<std::vector<El>> ab;
    combine(a, b, ab);
    return 2 * entropy_kl(ab) - entropy_kl(a) - entropy_kl(b);
}

template <typename V>
template <template <class, class> class Container, class Allocator_inner, class Allocator_outer, class El>
typename std::enable_if<!std::is_integral<El>::value, V>::type VOI_normalized_kl<V>::operator()(
    const Container<Container<El, Allocator_inner>, Allocator_outer>& a,
    const Container<Container<El, Allocator_inner>, Allocator_outer>& b) const
{
    auto entropy_a = entropy_kl(a);
    auto entropy_b = entropy_kl(b);
    std::vector<std::vector<El>> ab;
    combine(a, b, ab);
    auto joint_entropy = entropy_kl(ab);
    auto mi = entropy_a + entropy_b - joint_entropy;
    return 1 - (mi / (entropy_a + entropy_b - mi));
}








}  // namespace metric
#endif
