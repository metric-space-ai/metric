/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#ifndef _METRIC_DISTANCE_K_RANDOM_ENTROPY_CPP
#define _METRIC_DISTANCE_K_RANDOM_ENTROPY_CPP

//#include <algorithm>
#include <cmath>
//#include <iostream>
//#include <random>
//#include <unordered_set>
#include <vector>

//#include <boost/functional/hash.hpp>

//#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/digamma.hpp>
#include <boost/math/special_functions/gamma.hpp>

#include "../space/tree.hpp"
#include "estimator_helpers.cpp"
#include "epmgp.cpp"


namespace metric {




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



/* // good code, but all calls are disabled
template <template <typename, typename> class OuterContainer, typename Container, typename OuterAllocator>
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
*/


template <typename T1, typename T2>
T1 log(T1 logbase, T2 x)
{
    return std::log(x) / std::log(logbase);
}





double mvnpdf(blaze::DynamicVector<double> x, blaze::DynamicVector<double> mu, blaze::DynamicMatrix<double> Sigma) {

    size_t n = x.size();
    assert(mu.size() == n && Sigma.columns() == n && Sigma.rows() == n);

    auto centered = x - mu;
    auto p =  blaze::trans(centered) * blaze::inv(Sigma) * centered;
    //return std::exp(-p/2) / ( std::sqrt(blaze::det(Sigma)) * std::pow(2*M_PI, (double)n/2.0) );
    return std::exp(-p/2) / std::sqrt( blaze::det(Sigma) * std::pow(2*M_PI, n) );
}


double mvnpdf(blaze::DynamicVector<double> x) {

    return( mvnpdf(x, blaze::DynamicVector<double>(x.size(), 0), blaze::IdentityMatrix<double>(x.size())) );
}




// averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing

template <typename Container, typename EntropyFunctor, typename Metric>
double estimate_func(
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

    auto e = EntropyFunctor();

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







// ----------------------------------- entropy

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
    return estimate_func<Container, entropy<recType, Metric>, Metric>(a, sampleSize, threshold, maxIterations, k, logbase, metric, exp);

    /*
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
    */
}







// ----------------------------------- entropy_kpN

template <typename recType, typename Metric>
template <template <typename, typename> class OuterContainer, typename Container, typename OuterAllocator>
double entropy_kpN<recType, Metric>::operator()(
        const OuterContainer<Container, OuterAllocator> X,
        Metric metric, // = Metric(),
        size_t k, // = 7,
        size_t p // = 70
        ) const
{
    size_t n = X.size();
    size_t d = X[0].size();

    assert(p < n);
    assert(k < p);

    double h = 0;
    int got_results = 0;  // absents in Matlab original code

    metric::Tree<Container, Metric> tree (X, -1, metric);
    blaze::DynamicMatrix<double> Nodes (p, d, 0);
    blaze::DynamicVector<double> mu (d, 0);
    blaze::DynamicVector<double> lb (d, 0);
    blaze::DynamicVector<double> ub (d, 0);
    blaze::DynamicVector<double> x_vector (d, 0);
    for (size_t i = 0; i < n; ++i) {

        auto res = tree.knn(X[i], p);
        auto eps = res[k-1].second;

        blaze::reset(mu);
        for (size_t p_idx= 0; p_idx < p; ++p_idx) { // r v realizations from the tree
            for (size_t d_idx = 0; d_idx < d; ++d_idx) { // dimensions
                Nodes(p_idx, d_idx) = res[p_idx].first->data[d_idx];
                mu[d_idx] += res[p_idx].first->data[d_idx];
            }
        }
        mu = mu/p;
        Nodes = Nodes - blaze::expand(blaze::trans(mu), Nodes.rows());
        double offset = 1e-8;
        //double offset = 1e-5; // todo compute depending of machine epsilon
        auto K = blaze::evaluate( (blaze::trans(Nodes) * Nodes)*p/(p - 1) + blaze::IdentityMatrix<double>(d)*offset );

        blaze::reset(lb);
        blaze::reset(ub);
        for (size_t d_idx = 0; d_idx < d; ++d_idx) { // dimensions
            lb[d_idx] = X[i][d_idx] - eps;
            ub[d_idx] = X[i][d_idx] + eps;
            x_vector[d_idx] = X[i][d_idx];
        }

//        std::cout << "---- mu:\n" << mu << "\n"; // TODO remove
//        std::cout << "---- K:\n" << K << "\n";
//        std::cout << "---- lb:\n" << lb << "\n";
//        std::cout << "---- ub:\n" << ub << "\n";
        //std::cout << "---- Nodes:\n" << Nodes << "\n";
        //std::cout << "---- Cov:\n" << (blaze::trans(Nodes) * Nodes)/(p - 1) << "\n";

        auto g_local = epmgp::local_gaussian_axis_aligned_hyperrectangles<double>(mu, K, lb, ub);
        double logG = std::get<0>(g_local);

        if (!std::isnan(logG)) { // UNLIKE original Matlab code, we exclude points that result in NaN, TODO check math validity
            double g = mvnpdf(x_vector, mu, K);
            h += logG - std::log(g);
            got_results++;
        }

    }

    if (got_results > 20) // absents in Matlab original code
        return boost::math::digamma(n) - boost::math::digamma(k) + h/n;
    else
        return std::nan("not defined");
}



// averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing
template <typename recType, typename Metric>
template <typename Container>
double entropy_kpN<recType, Metric>::estimate(
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
    return estimate_func<Container, entropy_kpN<recType, Metric>, Metric>(a, sampleSize, threshold, maxIterations, k, logbase, metric, exp);

    /*
    const size_t dataSize = a.size();

    // Update maxIterations
    if (maxIterations == 0) {
        maxIterations = dataSize / sampleSize;
    }

    if (maxIterations > dataSize / sampleSize) {
        maxIterations = dataSize / sampleSize;
    }

    auto e = entropy_kpN<void, Metric>();

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
    */
}







// ---------------------------------- estimators to be debugged




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



// ported from Julia, not in use
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










// ----------------------------------- old versiions




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










} // namespace metric

#endif
