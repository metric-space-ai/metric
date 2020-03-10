/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/


#ifndef _ENTROPY_KPN_CPP
#define _ENTROPY_KPN_CPP

#define _USE_MATH_DEFINES

#include <boost/math/special_functions/digamma.hpp>

#include "epmgp.cpp"
#include "estimator_helpers.cpp"
#include "../../space/tree.hpp"


namespace metric {



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




//template <typename recType, typename Metric = metric::Chebyshev<typename recType::value_type>>
//double entropy_kpN(
//        std::vector<recType> X,
//        Metric metric = Metric(),
//        size_t k = 7,
//        size_t p = 70
//        )
//{

//    size_t n = X.size();
//    size_t d = X[0].size();

//    double h = 0;

//    metric::Tree<recType, Metric> tree (X, -1, metric);
//    blaze::DynamicMatrix<double> Nodes (p, d, 0);
//    blaze::DynamicVector<double> mu (d, 0);
//    blaze::DynamicVector<double> lb (d, 0);
//    blaze::DynamicVector<double> ub (d, 0);
//    blaze::DynamicVector<double> x_vector (d, 0);
//    for (size_t i = 0; i < n; ++i) {

//        auto res = tree.knn(X[i], p);
//        auto eps = res[k-1].second;

//        blaze::reset(mu);
//        for (size_t p_idx= 0; p_idx < p; ++p_idx) { // r v realizations from the tree
//            for (size_t d_idx = 0; d_idx < d; ++d_idx) { // dimensions
//                Nodes(p_idx, d_idx) = res[p_idx].first->data[d_idx];
//                mu[d_idx] += res[p_idx].first->data[d_idx];
//            }
//        }
//        mu = mu/p;
//        Nodes = Nodes - blaze::expand(blaze::trans(mu), Nodes.rows());
//        auto K = blaze::evaluate( (blaze::trans(Nodes) * Nodes)*p/(p - 1) + blaze::IdentityMatrix<double>(d)*double(1e-8) );

//        blaze::reset(lb);
//        blaze::reset(ub);
//        for (size_t d_idx = 0; d_idx < d; ++d_idx) { // dimensions
//            lb[d_idx] = X[i][d_idx] - eps;
//            ub[d_idx] = X[i][d_idx] + eps;
//            x_vector[d_idx] = X[i][d_idx];
//        }

//        //std::cout << "---- mu:\n" << mu << "\n"; // TODO remove
//        //std::cout << "---- K:\n" << K << "\n";
//        //std::cout << "---- lb:\n" << lb << "\n";
//        //std::cout << "---- ub:\n" << ub << "\n";
//        //std::cout << "---- Nodes:\n" << Nodes << "\n";
//        //std::cout << "---- Cov:\n" << (blaze::trans(Nodes) * Nodes)/(p - 1) << "\n";

//        auto g_local = epmgp::local_gaussian_axis_aligned_hyperrectangles<double>(mu, K, lb, ub);
//        double logG = std::get<0>(g_local);

//        double g = mvnpdf(x_vector, mu, K);

//        h += logG - std::log(g);
//    }

//    return boost::math::digamma(n) - boost::math::digamma(k) + h/n;
//}


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

    double h = 0;

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
        auto K = blaze::evaluate( (blaze::trans(Nodes) * Nodes)*p/(p - 1) + blaze::IdentityMatrix<double>(d)*double(1e-8) );

        blaze::reset(lb);
        blaze::reset(ub);
        for (size_t d_idx = 0; d_idx < d; ++d_idx) { // dimensions
            lb[d_idx] = X[i][d_idx] - eps;
            ub[d_idx] = X[i][d_idx] + eps;
            x_vector[d_idx] = X[i][d_idx];
        }

        //std::cout << "---- mu:\n" << mu << "\n"; // TODO remove
        //std::cout << "---- K:\n" << K << "\n";
        //std::cout << "---- lb:\n" << lb << "\n";
        //std::cout << "---- ub:\n" << ub << "\n";
        //std::cout << "---- Nodes:\n" << Nodes << "\n";
        //std::cout << "---- Cov:\n" << (blaze::trans(Nodes) * Nodes)/(p - 1) << "\n";

        auto g_local = epmgp::local_gaussian_axis_aligned_hyperrectangles<double>(mu, K, lb, ub);
        double logG = std::get<0>(g_local);

        double g = mvnpdf(x_vector, mu, K);

        h += logG - std::log(g);
    }

    return boost::math::digamma(n) - boost::math::digamma(k) + h/n;
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
    const size_t dataSize = a.size();

    /* Update maxIterations */
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




} // namespace metric


#endif  // _ENTROPY_KPN_CPP
