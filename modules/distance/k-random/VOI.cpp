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
#include "estimator_helpers.cpp"
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


//MOVED TO ENTROPY.*PP
/*
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
*/

//template <typename Container, typename Metric, typename L>
//double entropy_fn( // old erroneous version, TODO remove
//    std::vector<Container> data, std::size_t k, L logbase, Metric metric, bool exp)
//{
//    using T = typename Container::value_type;

//    if (data.empty() || data[0].empty()) {
//        return 0;
//    }
//    if (data.size() < k + 1)
//        throw std::invalid_argument("number of points in dataset must be larger than k");

//    double p = 1;
//    double N = data.size();
//    double d = data[0].size();
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

//    //add_noise(data); // TODO test
//    metric::Tree<Container, Metric> tree(data, -1, metric);
//    double entropyEstimate = boost::math::digamma(N) - boost::math::digamma(k) + cb + d * log(logbase, two);
//    for (std::size_t i = 0; i < N; i++) {
//        auto res = tree.knn(data[i], k + 1);
//        entropyEstimate += d / N * log(logbase, res.back().second);
//    }
//    if (exp)
//        return metric::conv_diff_entropy(entropyEstimate); // conversion of values below 1 to exp scale
//    else
//        return entropyEstimate;
//}


/* // original version, erroneous

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




/* // updated original version, tested, ok

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
    //double cb = d * log(logbase, two);
    double cb = 0;

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
    //double entropyEstimate = boost::math::digamma(N) - boost::math::digamma(k) + cb + d * log(logbase, two);
    double entropyEstimate = (boost::math::digamma(N) - boost::math::digamma(k)) / std::log(logbase) + cb + d * log(logbase, two);
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




/*
// updated version, results are correct only for Shebyshev metric!!
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
//*/



// MOVED TO ENTROPY.*PP
/*
// updated version, for different metric
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

    double N = data.size();
    double d = data[0].size();

    //add_noise(data); // TODO test
    metric::Tree<Container, Metric> tree(data, -1, metric);

    double entropyEstimate = 0;
    double log_sum = 0;

    for (std::size_t i = 0; i < N; i++) {
        auto res = tree.knn(data[i], k + 1);
        entropyEstimate += std::log(res.back().second);
    }
    entropyEstimate = entropyEstimate * d / (double)N; // mean log * d
    entropyEstimate += boost::math::digamma(N) - boost::math::digamma(k) + d*std::log(2.0);

    if constexpr (!std::is_same<Metric, typename metric::Chebyshev<T>>::value) {
        double p = 1; // Manhatten and other metrics (TODO check if it is correct for them!)
        if constexpr (std::is_same<Metric, typename metric::Euclidian<T>>::value) {
            p = 2; // Euclidean
        } else if constexpr (std::is_same<Metric, typename metric::P_norm<T>>::value) {
            p = metric.p; // general Minkowsky
        }
        entropyEstimate += d * std::log(std::tgamma(1 + 1 / p)) - std::log(std::tgamma(1 + d / p));
    }
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

    // Update maxIterations
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

    // Create shuffle indexes
    std::vector<size_t> indexes(dataSize);
    std::iota(indexes.begin(), indexes.end(), 0);

    auto rng = std::default_random_engine();
    std::shuffle(indexes.begin(), indexes.end(), rng);

    // Create vector container for fast random access
    const std::vector<typename Container::value_type> vectorA(a.begin(), a.end());

    // Create samples
    std::vector<typename Container::value_type> sampleA;
    sampleA.reserve(sampleSize);

    std::vector<double> entropyValues;
    double mu = 0;
    for (auto i = 1; i <= maxIterations; ++i) {
        size_t start = (i - 1) * sampleSize;
        size_t end = std::min(i * sampleSize - 1, dataSize - 1);

        // Create samples
        sampleA.clear();

        for (auto j = start; j < end; ++j) {
            sampleA.push_back(vectorA[indexes[j]]);
        }

        // Get sample mgc value
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

// */




// Kozachenko-Leonenko estimator based on https://hal.archives-ouvertes.fr/hal-00331300/document (Shannon diff. entropy,
// q = 1)
// WARNING: this estimator is sill under construction, it seems to have bugs and needs debugging

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
