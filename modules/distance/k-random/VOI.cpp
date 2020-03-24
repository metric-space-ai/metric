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

#include <boost/functional/hash.hpp>

#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/digamma.hpp>
#include <boost/math/special_functions/gamma.hpp>

#include "../../space/tree.hpp"
#include "VOI.hpp"

namespace metric {

namespace {
    template <typename C>
    void add_noise(C & data)
    {
        using T = type_traits::underlaying_type_t<C>;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<T> dis(0, 1);
        double c = 1e-10;
        for (auto& v : data) {
            std::transform(v.begin(), v.end(), v.begin(), [&gen, c, &dis](T e) {
                auto g = dis(gen);
                auto k = e + c * g;
                return k;
            });
        }
    }

    template <typename T1, typename T2>
    T1 log(T1 logbase, T2 x)
    {
        return std::log(x) / std::log(logbase);
    }

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


template <typename Container, typename Metric, typename L>
double entropy(
    const Container & data, std::size_t k, L logbase, const Metric & metric)
{
    using T_underlaying = type_traits::underlaying_type_t<Container>;
    using T = type_traits::index_value_type_t<Container>;
    if (data.size() == 0 || data[0].size() == 0) {
        return 0;
    }
    if (data.size() < k + 1)
        throw std::invalid_argument("number of points in dataset must be larger than k");

    double p = 1;
    double N = data.size();
    double d = data[0].size();
    double two = 2.0;  // this is in order to make types match the log template function
    double cb = d * log(logbase, two);

    if constexpr (!std::is_same<Metric, typename metric::Chebyshev<T_underlaying>>::value) {
        if constexpr (std::is_same<Metric, typename metric::Euclidian<T_underlaying>>::value) {
            p = 2;
        } else if constexpr (std::is_same<Metric, typename metric::P_norm<T_underlaying>>::value) {
            p = metric.p;
        }
        cb = cb + d * log(logbase, std::tgamma(1 + 1 / p)) - log(logbase, std::tgamma(1 + d / p));
    }

    //add_noise(data); // TODO test
    metric::Tree<T, Metric> tree(data, -1, metric);
    double entropyEstimate = boost::math::digamma(N) - boost::math::digamma(k) + cb + d * log(logbase, two);
    for (std::size_t i = 0; i < N; i++) {
        auto res = tree.knn(data[i], k + 1);
        entropyEstimate += d / N * log(logbase, res.back().second);
    }
    return entropyEstimate;
}

// Kozachenko-Leonenko estimator based on https://hal.archives-ouvertes.fr/hal-00331300/document (Shannon diff. entropy,
// q = 1)

template <typename Container, typename Metric = metric::Euclidian<type_traits::underlaying_type_t<Container>>,
    typename L = type_traits::underlaying_type_t<Container>>  // TODO check if L = T is correct
std::enable_if_t<!type_traits::is_container_of_integrals_v<Container>, L>
entropy_kl( const Container & data, std::size_t k = 3, L logbase = 2, const Metric &metric = Metric())
{
    using T1 = type_traits::index_value_type_t<Container>;
    using T = type_traits::underlaying_type_t<Container>;
    if (data.empty() || data[0].empty())
        return 0;
    if (data.size() < k + 1)
        throw std::invalid_argument("number of points in dataset must be larger than k");
    if constexpr (!std::is_same<Metric, typename metric::Euclidian<T>>::value)
        throw std::logic_error("entropy function is now implemented only for Euclidean distance");

    metric::Tree<T1, Metric> tree(data, -1, metric);

    size_t N = data.size();
    size_t m = data[0].size();
    T two = 2.0;  // this is in order to make types match the log template function
    T sum = 0;
    auto Pi = boost::math::constants::pi<T>();
    T half_m = m / two;
    auto coeff = (N - 1) * exp(-boost::math::digamma(k + 1)) * std::pow(Pi, half_m) / boost::math::tgamma(half_m + 1);

    for (std::size_t i = 0; i < N; i++) {
        auto neighbors = tree.knn(data[i], k + 1);
        auto ro = neighbors.back().second;
        sum = sum + log(logbase, coeff * std::pow(ro, m));
    }

    return sum;
}

template <typename Container, typename T>
std::pair<std::vector<double>, Container> pluginEstimator(const Container & Y)
{
    std::vector<std::vector<T>> uniqueVal = unique(Y);
    std::vector<double> counts(uniqueVal.size());
    for (std::size_t i = 0; i < counts.size(); i++) {
        for (std::size_t j = 0; j < Y.size(); j++) {
            if (Y[j] == uniqueVal[i])
                counts[i]++;
        }
    }
    std::size_t length = Y.size() * Y[0].size();
    std::transform(counts.begin(), counts.end(), counts.begin(), [&length](auto& i) { return i / length; });

    return std::make_pair(counts, uniqueVal);
}

template <typename Container, typename Metric>
typename std::enable_if_t<!type_traits::is_container_of_integrals_v<Container>,
                          type_traits::underlaying_type_t<Container>>
mutualInformation(
                  const Container & Xc, const Container & Yc, int k, const Metric & metric, int version)
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

    add_noise(X);
    add_noise(Y);
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
            throw std::runtime_error("this version not allowed");
        }
        entropyEstimate -= 1.0 / N * boost::math::digamma(static_cast<double>(nx));
    }
    return entropyEstimate;
}

template <typename Container, typename T>
std::enable_if_t<type_traits::is_container_of_integrals_v<Container>, T>
mutualInformation(const Container& Xc, const Container& Yc, T logbase)
{
    std::vector<std::vector<T>> XY = combine(Xc, Yc);
    return entropy(Xc, logbase)
        + entropy(Yc,
            logbase)  // entropy overload for integers is not implemented yet
        - entropy(XY, logbase);
}

template <typename C, typename Metric, typename T>
typename std::enable_if_t<!type_traits::is_container_of_integrals_v<C>, T>
variationOfInformation(const C& Xc, const C& Yc, int k, T logbase)
{
    return entropy<C, Metric>(Xc, k, logbase, Metric()) + entropy<C, Metric>(Yc, k, logbase, Metric())
        - 2 * mutualInformation<C>(Xc, Yc, k);
}

template <typename C, typename T>
typename std::enable_if_t<!type_traits::is_container_of_integrals_v<C>, T>
variationOfInformation_normalized(
        const C& Xc, const C& Yc, int k, T logbase)
{
    using Cheb = metric::Chebyshev<T>;
    auto mi = mutualInformation<C>(Xc, Yc, k);
    return 1 - (mi / (entropy(Xc, k, logbase, Cheb()) + entropy(Yc, k, logbase, Cheb()) - mi));
}

template <typename V>
template <typename C, typename El>
typename std::enable_if_t<!std::is_integral_v<El>, V>  // only real values are accepted
VOI<V>::operator()(const C& a, const C& b) const
{
    using Cheb = metric::Chebyshev<El>;
    return entropy(a, k, logbase, Cheb()) + entropy(b, k, logbase, Cheb())
        - 2 * mutualInformation<C>(a, b, k);
}

template <typename V>
template <typename C, typename El>
typename std::enable_if_t<std::is_integral_v<El>, V>  // only real values are accepted
VOI<V>::operator()(const C& a, const C& b) const
{
    using Cheb = metric::Chebyshev<El>;
    return entropy(a, k, logbase, Cheb()) + entropy(b, k, logbase, Cheb())
        - 2 * mutualInformation<C>(a, b, k);
}

template <typename V>
template <typename C, typename El>
typename std::enable_if_t<!std::is_integral_v<El>, V>  // only real values are accepted
VOI_normalized<V>::operator()(const C& a, const C& b) const
{
    using Cheb = metric::Chebyshev<El>;
    auto mi = mutualInformation(a, b, this->k);
    return 1
        - (mi
            / (entropy(a, this->k, this->logbase, Cheb())
                + entropy(b, this->k, this->logbase, Cheb()) - mi));
}

// VOI based on Kozachenko-Leonenko entropy estimator

template <typename V>
template <typename C, typename El>
typename std::enable_if_t<!std::is_integral_v<El>, V>  // only real values are accepted
    VOI_kl<V>::operator()(const C& a, const C& b) const
{
    std::vector<std::vector<El>> ab = combine(a, b);
    return 2 * entropy_kl(ab) - entropy_kl(a) - entropy_kl(b);
}

template <typename V>
template <typename C, typename El>
typename std::enable_if_t<!std::is_integral_v<El>, V>  // only real values are accepted
VOI_normalized_kl<V>::operator()(const C& a, const C& b) const
{
    auto entropy_a = entropy_kl(a);
    auto entropy_b = entropy_kl(b);
    std::vector<std::vector<El>> ab = combine(a, b);
    auto joint_entropy = entropy_kl(ab);
    auto mi = entropy_a + entropy_b - joint_entropy;
    return 1 - (mi / (entropy_a + entropy_b - mi));
}

}  // namespace metric
#endif
