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
#include <boost/math/special_functions/gamma.hpp>

#include "../../space/tree.hpp"
#include "VOI.hpp"

namespace metric {

namespace {
    //template <typename T>
    template <template <typename, typename> class OuterContainer, typename Container, typename OuterAllocator>
    //void add_noise(std::vector<std::vector<T>>& data)
    void add_noise(OuterContainer<Container, OuterAllocator> & data)
    {
        using T = typename Container::value_type;

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



template <typename T>
T conv_diff_entropy(T in) {
    if (in < 1)
        return 1/std::exp(1)*std::exp(in);
    else
        return in;
}

template <typename T>
T conv_diff_entropy_inv(T in) {
    if (in < 1)
        return std::log(in) + 1;
    else
        return in;
}


template <typename Container, typename Metric, typename L>
double entropy_fn( // old version, TODO remove
    std::vector<Container> data, std::size_t k, L logbase, Metric metric, bool exp)
{
    using T = typename Container::value_type;

    if (data.empty() || data[0].empty()) {
        return 0;
    }
    if (data.size() < k + 1)
        throw std::invalid_argument("number of points in dataset must be larger than k");

    double p = 1;
    double N = data.size();
    double d = data[0].size();
    double two = 2.0;  // this is in order to make types match the log template function
    double cb = d * log(logbase, two);

    if constexpr (!std::is_same<Metric, typename metric::Chebyshev<T>>::value) {
        if constexpr (std::is_same<Metric, typename metric::Euclidian<T>>::value) {
            p = 2;
        } else if constexpr (std::is_same<Metric, typename metric::P_norm<T>>::value) {
            p = metric.p;
        }
        cb = cb + d * log(logbase, std::tgamma(1 + 1 / p)) - log(logbase, std::tgamma(1 + d / p));
    }

    //add_noise(data); // TODO test
    metric::Tree<Container, Metric> tree(data, -1, metric);
    double entropyEstimate = boost::math::digamma(N) - boost::math::digamma(k) + cb + d * log(logbase, two);
    for (std::size_t i = 0; i < N; i++) {
        auto res = tree.knn(data[i], k + 1);
        entropyEstimate += d / N * log(logbase, res.back().second);
    }
    if (exp)
        return metric::conv_diff_entropy(entropyEstimate); // conversion of values below 1 to exp scale
    else
        return entropyEstimate;
}


/* // original version

// averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing
template <typename recType, typename Metric>
template <template <typename, typename> class OuterContainer, typename Container, typename OuterAllocator>
double entropy<recType, Metric>::operator()(
        const OuterContainer<Container, OuterAllocator> & data,
        std::size_t k,
        double logbase,
        Metric metric,
        bool exp
        ) const
{
    using T = typename Container::value_type;

    if (data.empty() || data[0].empty()) {
        return 0;
    }
    if (data.size() < k + 1)
        throw std::invalid_argument("number of points in dataset must be larger than k");

    double p = 1;
    double N = data.size();
    double d = data[0].size();
    double two = 2.0;  // this is in order to make types match the log template function
    double cb = d * log(logbase, two);

    if constexpr (!std::is_same<Metric, typename metric::Chebyshev<T>>::value) {
        if constexpr (std::is_same<Metric, typename metric::Euclidian<T>>::value) {
            p = 2;
        } else if constexpr (std::is_same<Metric, typename metric::P_norm<T>>::value) {
            p = metric.p;
        }
        cb = cb + d * log(logbase, std::tgamma(1 + 1 / p)) - log(logbase, std::tgamma(1 + d / p));
    }

    //add_noise(data); // TODO test
    metric::Tree<Container, Metric> tree(data, -1, metric);
    double entropyEstimate = boost::math::digamma(N) - boost::math::digamma(k) + cb + d * log(logbase, two);
    for (std::size_t i = 0; i < N; i++) {
        auto res = tree.knn(data[i], k + 1);
        entropyEstimate += d / N * log(logbase, res.back().second);
    }
    if (exp)
        return metric::conv_diff_entropy(entropyEstimate); // conversion of values below 1 to exp scale
    else
        return entropyEstimate;
}

//*/



// updated version
// averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing
template <typename recType, typename Metric>
template <template <typename, typename> class OuterContainer, typename Container, typename OuterAllocator>
double entropy<recType, Metric>::operator()(
        const OuterContainer<Container, OuterAllocator> & data,
        std::size_t k,
        double logbase,
        Metric metric,
        bool exp
        ) const
{
    using T = typename Container::value_type;

    if (data.empty() || data[0].empty()) {
        return 0;
    }
    if (data.size() < k + 1)
        throw std::invalid_argument("number of points in dataset must be larger than k");

    double p = 1;
    double N = data.size();
    double d = data[0].size();
//    double two = 2.0;  // this is in order to make types match the log template function
//    double cb = d * log(logbase, two);

//    if constexpr (!std::is_same<Metric, typename metric::Chebyshev<T>>::value) {
//        if constexpr (std::is_same<Metric, typename metric::Euclidian<T>>::value) {
//            p = 2;
//        } else if constexpr (std::is_same<Metric, typename metric::P_norm<T>>::value) {
//            p = metric.p;
//        }
//        cb = cb + d * log(logbase, std::tgamma(1 + 1 / p)) - log(logbase, std::tgamma(1 + d / p));
//    }

    //add_noise(data); // TODO test
    metric::Tree<Container, Metric> tree(data, -1, metric);
//    double entropyEstimate = boost::math::digamma(N) - boost::math::digamma(k) + cb + d * log(logbase, two);

    double entropyEstimate = 0;
    double log_sum = 0;

    for (std::size_t i = 0; i < N; i++) {
        auto res = tree.knn(data[i], k + 1);
        //entropyEstimate += d / N * log(logbase, res.back().second);
        entropyEstimate += std::log(res.back().second);
    }
    entropyEstimate = entropyEstimate * d / (double)N; // mean log * d
    entropyEstimate += boost::math::digamma(N) - boost::math::digamma(k) + d*std::log(2.0);
    entropyEstimate /= std::log(logbase);
    if (exp)
        return metric::conv_diff_entropy(entropyEstimate); // conversion of values below 1 to exp scale
    else
        return entropyEstimate;
}



// averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing
template <typename recType, typename Metric>
template <typename Container>
double entropy<recType, Metric>::estimate(
        const Container & a,
        const size_t sampleSize,
        const double threshold,
        size_t maxIterations,
        std::size_t k,
        double logbase,
        Metric metric,
        bool exp // TODO apply to returning value!
        )
{
    const size_t dataSize = a.size();

    /* Update maxIterations */
    if (maxIterations == 0) {
        maxIterations = dataSize / sampleSize;
    }

    if (maxIterations > dataSize / sampleSize) {
        maxIterations = dataSize / sampleSize;
    }

    auto e = entropy<void, Metric>();

    if (maxIterations < 1) {
        return e(a, k, logbase, metric);
    }

    /* Create shuffle indexes */
    std::vector<size_t> indexes(dataSize);
    std::iota(indexes.begin(), indexes.end(), 0);

    auto rng = std::default_random_engine();
    std::shuffle(indexes.begin(), indexes.end(), rng);

    /* Create vector container for fast random access */
    const std::vector<typename Container::value_type> vectorA(a.begin(), a.end());

    /* Create samples */
    std::vector<typename Container::value_type> sampleA;
    sampleA.reserve(sampleSize);

    std::vector<double> entropyValues;
    double mu = 0;
    for (auto i = 1; i <= maxIterations; ++i) {
        size_t start = (i - 1) * sampleSize;
        size_t end = std::min(i * sampleSize - 1, dataSize - 1);

        /* Create samples */
        sampleA.clear();

        for (auto j = start; j < end; ++j) {
            sampleA.push_back(vectorA[indexes[j]]);
        }

        /* Get sample mgc value */
        double sample_entopy = e(sampleA, k, logbase, metric);
        entropyValues.push_back(sample_entopy);

        std::sort(entropyValues.begin(), entropyValues.end());

        const size_t n = entropyValues.size();
        const auto p0 = linspace(0.5 / n, 1 - 0.5 / n, n);

        mu = mean(entropyValues);
        double sigma = variance(entropyValues, mu);

        const std::vector<double> synth = icdf(p0, mu, sigma);
        std::vector<double> diff;
        diff.reserve(n);
        for (auto i = 0; i < n; ++i) {
            diff.push_back(entropyValues[i] - synth[i]);
        }

        auto convergence = peak2ems(diff) / n;
        std::cout << n << " " << convergence << " " << sample_entopy << " " << mu << std::endl;

        if (convergence < threshold) {
            return mu;
        }
    }

    return mu;
}



// Kozachenko-Leonenko estimator based on https://hal.archives-ouvertes.fr/hal-00331300/document (Shannon diff. entropy,
// q = 1)

template <typename Container, typename Metric = metric::Euclidian<typename Container::value_type>, typename L = double>  // TODO check if L = T is correct
typename std::enable_if<!std::is_integral<typename Container::value_type>::value, double>::type entropy_kl(
    std::vector<Container> data, std::size_t k = 3, L logbase = 2, Metric metric = Metric())
{
    using T = typename Container::value_type;

    if (data.empty() || data[0].empty())
        return 0;
    if (data.size() < k + 1)
        throw std::invalid_argument("number of points in dataset must be larger than k");
    if constexpr (!std::is_same<Metric, typename metric::Euclidian<T>>::value)
        throw std::logic_error("entropy function is now implemented only for Euclidean distance");

    metric::Tree<Container, Metric> tree(data, -1, metric);

    size_t N = data.size();
    size_t m = data[0].size();
    double two = 2.0;  // this is in order to make types match the log template function
    double sum = 0;
    auto Pi = boost::math::constants::pi<T>();
    double half_m = m / two;
    auto coeff = (N - 1) * exp(-boost::math::digamma(k + 1)) * std::pow(Pi, half_m) / boost::math::tgamma(half_m + 1);

    for (std::size_t i = 0; i < N; i++) {
        auto neighbors = tree.knn(data[i], k + 1);
        auto ro = neighbors.back().second;
        sum = sum + log(logbase, coeff * std::pow(ro, m));
    }

    return sum;
} // TODO debug!!


template <typename T>
std::pair<std::vector<double>, std::vector<std::vector<T>>> pluginEstimator(const std::vector<std::vector<T>>& Y)
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



// averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing


// helper functions



std::vector<double> linspace(double a, double b, int n)
{
    std::vector<double> array;
    if (n > 1) {
        double step = (b - a) / double(n - 1);
        int count = 0;
        while (count < n) {
            array.push_back(a + count * step);
            ++count;
        }
    } else {
        array.push_back(b);
    }

    return array;
}



double polyeval(const std::vector<double>& poly, const double z)
{
    const int n = poly.size();
    double sum = poly[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        sum *= z;
        sum += poly[i];
    }
    return sum;
}



double erfcinv(const double z)
{
    if ((z < 0) || (z > 2))
        std::cout << "Argument outside range [0,2] in inverse erfc function (got p=%1%)." << std::endl;

    double p, q, s;
    if (z > 1) {
        q = 2 - z;
        p = 1 - q;
        s = -1;
    } else {
        p = 1 - z;
        q = z;
        s = 1;
    }
    return s * erfinv_imp(p, q);
}


double erfinv_imp(const double p, const double q)
{
    double result = 0;

    if (p <= 0.5) {
        static const float Y = 0.0891314744949340820313f;
        static const std::vector<double> P = { -0.000508781949658280665617L, -0.00836874819741736770379L,
            0.0334806625409744615033L, -0.0126926147662974029034L, -0.0365637971411762664006L,
            0.0219878681111168899165L, 0.00822687874676915743155L, -0.00538772965071242932965L };
        static const std::vector<double> Q = { 1, -0.970005043303290640362L, -1.56574558234175846809L,
            1.56221558398423026363L, 0.662328840472002992063L, -0.71228902341542847553L, -0.0527396382340099713954L,
            0.0795283687341571680018L, -0.00233393759374190016776L, 0.000886216390456424707504L };

        double g = p * (p + 10);
        double r = polyeval(P, p) / polyeval(Q, p);
        result = g * Y + g * r;
    } else if (q >= 0.25) {
        static const float Y = 2.249481201171875f;
        static const std::vector<double> P = { -0.202433508355938759655L, 0.105264680699391713268L,
            8.37050328343119927838L, 17.6447298408374015486L, -18.8510648058714251895L, -44.6382324441786960818L,
            17.445385985570866523L, 21.1294655448340526258L, -3.67192254707729348546L };
        static const std::vector<double> Q
            = { 1L, 6.24264124854247537712L, 3.9713437953343869095L, -28.6608180499800029974L, -20.1432634680485188801L,
                  48.5609213108739935468L, 10.8268667355460159008L, -22.6436933413139721736L, 1.72114765761200282724L };
        double g = std::sqrt(-2 * std::log(q));
        double xs = q - 0.25;
        double r = polyeval(P, xs) / polyeval(Q, xs);
        result = g / (Y + r);
    } else {
        double x = std::sqrt(-std::log(q));
        if (x < 3) {
            static const float Y = 0.807220458984375f;
            static const std::vector<double> P = { -0.131102781679951906451L, -0.163794047193317060787L,
                0.117030156341995252019L, 0.387079738972604337464L, 0.337785538912035898924L, 0.142869534408157156766L,
                0.0290157910005329060432L, 0.00214558995388805277169L, -0.679465575181126350155e-6L,
                0.285225331782217055858e-7L, -0.681149956853776992068e-9L };
            static const std::vector<double> Q = { 1, 3.46625407242567245975L, 5.38168345707006855425L,
                4.77846592945843778382L, 2.59301921623620271374L, 0.848854343457902036425L, 0.152264338295331783612L,
                0.01105924229346489121L };
            double xs = x - 1.125;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else if (x < 6) {
            static const float Y = 0.93995571136474609375f;
            static const std::vector<double> P
                = { -0.0350353787183177984712L, -0.00222426529213447927281L, 0.0185573306514231072324L,
                      0.00950804701325919603619L, 0.00187123492819559223345L, 0.000157544617424960554631L,
                      0.460469890584317994083e-5L, -0.230404776911882601748e-9L, 0.266339227425782031962e-11L };
            static const std::vector<double> Q
                = { 1L, 1.3653349817554063097L, 0.762059164553623404043L, 0.220091105764131249824L,
                      0.0341589143670947727934L, 0.00263861676657015992959L, 0.764675292302794483503e-4L };
            double xs = x - 3;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else if (x < 18) {
            static const float Y = 0.98362827301025390625f;
            static const std::vector<double> P
                = { -0.0167431005076633737133L, -0.00112951438745580278863L, 0.00105628862152492910091L,
                      0.000209386317487588078668L, 0.149624783758342370182e-4L, 0.449696789927706453732e-6L,
                      0.462596163522878599135e-8L, -0.281128735628831791805e-13L, 0.99055709973310326855e-16L };
            static const std::vector<double> Q
                = { 1L, 0.591429344886417493481L, 0.138151865749083321638L, 0.0160746087093676504695L,
                      0.000964011807005165528527L, 0.275335474764726041141e-4L, 0.282243172016108031869e-6L };
            double xs = x - 6;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else if (x < 44) {
            static const float Y = 0.99714565277099609375f;
            static const std::vector<double> P = { -0.0024978212791898131227L, -0.779190719229053954292e-5L,
                0.254723037413027451751e-4L, 0.162397777342510920873e-5L, 0.396341011304801168516e-7L,
                0.411632831190944208473e-9L, 0.145596286718675035587e-11L, -0.116765012397184275695e-17L };
            static const std::vector<double> Q
                = { 1L, 0.207123112214422517181L, 0.0169410838120975906478L, 0.000690538265622684595676L,
                      0.145007359818232637924e-4L, 0.144437756628144157666e-6L, 0.509761276599778486139e-9L };
            double xs = x - 18;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else {
            static const float Y = 0.99941349029541015625f;
            static const std::vector<double> P = { -0.000539042911019078575891L, -0.28398759004727721098e-6L,
                0.899465114892291446442e-6L, 0.229345859265920864296e-7L, 0.225561444863500149219e-9L,
                0.947846627503022684216e-12L, 0.135880130108924861008e-14L, -0.348890393399948882918e-21L };
            static const std::vector<double> Q
                = { 1L, 0.0845746234001899436914L, 0.00282092984726264681981L, 0.468292921940894236786e-4L,
                      0.399968812193862100054e-6L, 0.161809290887904476097e-8L, 0.231558608310259605225e-11L };
            double xs = x - 44;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        }
    }
    return result;
}



std::vector<double> icdf(const std::vector<double>& prob, const double mu, const double sigma)
{
    std::vector<double> synth;
    synth.reserve(prob.size());
    for (auto p : prob) {
        synth.push_back(mu + -1.41421356237309504880 * erfcinv(2 * p) * sigma);
    }

    return synth;
}



double variance(const std::vector<double>& data, const double mean)
{
    double sum = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        sum += (data[i] - mean) * (data[i] - mean);
    }

    return sum;
}



double mean(const std::vector<double>& data)
{
    double sum = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        sum += data[i];
    }
    double value = sum / data.size();

    return value;
}


double peak2ems(const std::vector<double>& data)
{
    double maxAbs = -1;
    double rms = 0;
    for (const auto v : data) {
        const double absV = abs(v);
        if (absV > maxAbs) {
            maxAbs = absV;
        }

        rms += v * v;
    }

    rms /= data.size();
    rms = sqrt(rms);

    return maxAbs / rms;
}



//// entropy estimation function

////template <typename Container, class Metric>
//template <typename Container, typename Metric, typename L>
//double entropy_avg(
//        const Container & a,
//        const size_t sampleSize,
//        const double threshold,
//        size_t maxIterations,
//        std::size_t k,
//        L logbase,
//        Metric metric,
//        bool exp)
//{
//    const size_t dataSize = a.size();

//    /* Update maxIterations */
//    if (maxIterations == 0) {
//        maxIterations = dataSize / sampleSize;
//    }

//    if (maxIterations > dataSize / sampleSize) {
//        maxIterations = dataSize / sampleSize;
//    }

//    if (maxIterations < 1) {
//        return entropy_fn(a);
//    }

//    /* Create shuffle indexes */
//    std::vector<size_t> indexes(dataSize);
//    std::iota(indexes.begin(), indexes.end(), 0);

//    auto rng = std::default_random_engine();
//    std::shuffle(indexes.begin(), indexes.end(), rng);

//    /* Create vector container for fast random access */
//    const std::vector<typename Container::value_type> vectorA(a.begin(), a.end());

//    /* Create samples */
//    std::vector<typename Container::value_type> sampleA;
//    sampleA.reserve(sampleSize);

//    std::vector<double> entropyValues;
//    double mu = 0;
//    for (auto i = 1; i <= maxIterations; ++i) {
//        size_t start = (i - 1) * sampleSize;
//        size_t end = std::min(i * sampleSize - 1, dataSize - 1);

//        /* Create samples */
//        sampleA.clear();

//        for (auto j = start; j < end; ++j) {
//            sampleA.push_back(vectorA[indexes[j]]);
//        }

//        /* Get sample mgc value */
//        double sample_entopy = entropy_fn(sampleA); // TODO add parameters
//        entropyValues.push_back(sample_entopy);

//        std::sort(entropyValues.begin(), entropyValues.end());

//        const size_t n = entropyValues.size();
//        const auto p0 = linspace(0.5 / n, 1 - 0.5 / n, n);

//        mu = mean(entropyValues);
//        double sigma = variance(entropyValues, mu);

//        const std::vector<double> synth = icdf(p0, mu, sigma);
//        std::vector<double> diff;
//        diff.reserve(n);
//        for (auto i = 0; i < n; ++i) {
//            diff.push_back(entropyValues[i] - synth[i]);
//        }

//        auto convergence = peak2ems(diff) / n;
//        std::cout << n << " " << convergence << " " << sample_entopy << " " << mu << std::endl;

//        if (convergence < threshold) {
//            return mu;
//        }
//    }

//    return mu;
//}






}  // namespace metric
#endif
