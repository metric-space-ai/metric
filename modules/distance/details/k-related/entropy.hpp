#ifndef _METRIC_DISTANCE_ENTROPY_HPP
#define _METRIC_DISTANCE_ENTROPY_HPP
/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include <random>
#include <cmath>

// for digamma function
#include <boost/math/special_functions/digamma.hpp>

//#include <boost/container_hash/hash.hpp> // replaced with <boost/functional/hash.hpp>, TODO check
#include <boost/functional/hash.hpp>

//#include "3dparty/metric_space/details/tree.hpp"
#include "../../../space/details/tree.hpp"
//#include "3dparty/metric_distance/details/Standards.hpp"
#include "../../../distance/details/k-related/Standards.hpp"
//#include "3dparty/metric_distance/details/chebyshev.hpp"
#include "chebyshev.hpp"

template<typename T>
void add_noise(std::vector<std::vector<T>> & data) {
    std::random_device rd;
    std::mt19937 gen(rd());
//    std::uniform_real_distribution<> dis(0, 1);
    std::normal_distribution<T> dis(0, 1); // replaced by Max F according to Julia code
    T c = 1e-10;
    for(auto & v : data) {
        std::transform(v.begin(), v.end(), v.begin(),
                       [&gen,c, &dis] (T e) {
                           auto g = dis(gen);
                           auto k = e + c*g;
                           return k;
                       });
    }
}
template<typename T>
T log(T logbase, T x) {
    return std::log(x) / std::log(logbase);
}


template<typename T>
void print_vec(const std::vector<T> & v) {
    std::cout << "[";
    for(auto d : v) {
        std::cout << d << ", ";
    }
    std::cout << "]";
}



template<typename Node_ptr, typename Distance>
void print(const std::vector<std::pair<Node_ptr,Distance>> & data) {
    std::cout << "[";
    for(auto & v : data) {
        print_vec(v.first->data);
        std::cout << " dist=" << v.second << "]";
    }
    std::cout << "]" << std::endl;
}



template <typename T, typename Metric>
typename std::enable_if<!std::is_integral<T>::value, T>::type  // replaced T with conditional type by Max F
entropy(std::vector<std::vector<T>> data, std::size_t k = 3, T logbase = 2, Metric metric = metric::distance::Euclidian<T>()) {
    if(data.empty() || data[0].empty()) {
        return 0;
    }
    T p = 1; // replaced double with T by Max F
    T N = data.size();
    T d = data[0].size();
    T two = 2.0; // this is in order to make types match the log template function
    T cb = d * log(logbase, two);

    if constexpr (!std::is_same<Metric, typename metric::distance::Chebyshev<T>>::value) {
        if constexpr ( std::is_same<Metric, typename metric::distance::Euclidian<T>>::value) {
            p = 2;
        } else if constexpr (std::is_same<Metric, typename metric::distance::P_norm<T>>::value) {
            p = metric.p;
        }
        cb = cb + d*log(logbase, std::tgamma(1+1/p)) - log(logbase, std::tgamma(1+d/p));
    }

    add_noise(data);
    metric_space::Tree<std::vector<T>, Metric> tree(data,-1, metric);
    //double cb = d * log(logbase,2.0) + d*log(logbase, std::tgamma(1+1/p)) - log(logbase, std::tgamma(1+d/p));
    T entropyEstimate = boost::math::digamma(N) - boost::math::digamma(k) // replaced double with T by Max F
        + cb + d * log(logbase, two);
    for(std::size_t i = 0; i < N; i++) {
        auto res = tree.knn(data[i],k+1);
        // print(res);
        // std::cout << "entropyEstimate=" << entropyEstimate << std::endl;
        // std::cout << "last_dist=" << res.back().second << std::endl;
        //std::cout << " -- " <<  res.back().second << "\n"; // TODO remove

        entropyEstimate += d/N*log(logbase,res.back().second);
    }
    return entropyEstimate;
}


// overload for integer types // added by Max F
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
entropy(std::vector<std::vector<T>> data, T logbase = 2)
{
    throw std::logic_error("entropy function is not implemented yet for integer data types");
    //return 0;
    // TODO implement using pluginEstimator
}


template<typename T>
std::vector<T> unique(const std::vector<T> & data) {
    std::unordered_set<std::size_t> hashes;
    std::vector<T> result;
    result.reserve(data.size());
    std::copy_if(data.begin(), data.end(), std::back_inserter(result),
                 [&hashes](const T &i) {
                     return hashes.insert(boost::hash_value(i)).second;
                 });
    return result;
}


template<typename T>
std::pair<std::vector<double>,std::vector<std::vector<T>>> // T replaced with double by Max F
pluginEstimator(const std::vector<std::vector<T>> & Y) {
    std::vector<std::vector<T>> uniqueVal = unique(Y);
    std::vector<double> counts(uniqueVal.size()); // T replaced with double by Max F in order to divide correctly in transform's lambda
    for(std::size_t i = 0; i < counts.size(); i++) {
        for(std::size_t j = 0; j < Y.size(); j++) {
            if(Y[j] == uniqueVal[i])
                counts[i]++;
        }
    }
    std::size_t length = Y.size() * Y[0].size();
    std::transform(counts.begin(), counts.end(), counts.begin(), [&length](auto & i) { return i/length;});

    return std::make_pair(counts, uniqueVal);
}


template <typename T>
void combine (const std::vector<std::vector<T>> & X, const std::vector<std::vector<T>> & Y, std::vector<std::vector<T>> &XY) {
    std::size_t N = X.size(); // replaced T with size_t by Max F
    std::size_t dx = X[0].size();
    std::size_t dy = Y[0].size();
    XY.resize(N);
    for(std::size_t i = 0; i < N; i++) {
        XY[i].resize(dx+dy);
        std::size_t k = 0;
        for(std::size_t j = 0; j < dx; j++, k++) {
            XY[i][k]= X[i][j];
        }
        for(std::size_t j = 0; j < dy; j++, k++) {
            XY[i][k]= Y[i][j];
        }
    }
}



template<typename T, typename Metric = metric::distance::Chebyshev<T>>
typename std::enable_if<!std::is_integral<T>::value, T>::type // line added by Max F
mutualInformation(const std::vector<std::vector<T>> & Xc,
                    const std::vector<std::vector<T>> & Yc,
                    int k = 3,  Metric metric = Metric(), int version = 2) {
    T N = Xc.size();

    auto X = Xc;
    auto Y = Yc;
    add_noise(X);
    add_noise(Y);
    std::vector<std::vector<T>> XY;
    combine(X,Y,XY);
    // XY.resize(N);
    // for(std::size_t i = 0; i < N; i++) {
    //     XY[i].resize(dx+dy);
    //     std::size_t k = 0;
    //     for(std::size_t j = 0; j < dx; j++, k++) {
    //         XY[i][k]= X[i][j];
    //     }
    //     for(std::size_t j = 0; j < dy; j++, k++) {
    //         XY[i][k]= Y[i][j];
    //     }
    // }
    metric_space::Tree<std::vector<T>, Metric> tree(XY,-1,metric);
    auto entropyEstimate = boost::math::digamma(k) + boost::math::digamma(N);
    //std::cout << "entropyEstimate1=" << entropyEstimate << std::endl;
    if(version == 2) {
        entropyEstimate -= 1/static_cast<double>(k);
    }
    //std::cout << "entropyEstimate2=" << entropyEstimate << std::endl;
    metric_space::Tree<std::vector<T>, Metric> xTree(X,-1,metric);
//    metric_space::Tree<std::vector<T>, Metric> yTree(Y,-1,metric); // never used, disabled by Max F
    for(std::size_t i = 0; i < N; i++) {
        auto res = tree.knn(XY[i],k+1);
        auto neighbor = res.back().first;
        auto dist = res.back().second;
        std::size_t nx = 0;
        if(version == 1) {
            auto dist_eps = std::nextafter(dist, std::numeric_limits<decltype(dist)>::max()); // this is instead of replacing < with <= in Tree // added by Max F in order to match Julia code logic without updating Tree
            nx = xTree.rnn(X[i], dist_eps).size(); // replaced dist by dist_eps by Max F
        } else if(version == 2) {
            auto ex = metric(X[neighbor->ID],X[i]);
            auto ex_eps = std::nextafter(ex, std::numeric_limits<decltype(ex)>::max()); // this it to include the most distant point into the sphere // added by Max F in order to match Julia code logic without updating Tree
            //nx = xTree.rnn(X[i], ex_eps).size(); // replaced ex by ex_eps by Max F //    TODO ebable

            //  debug code
            auto rnn_set = xTree.rnn(X[i], ex_eps);
            nx = rnn_set.size(); // replaced ex by ex_eps by Max F


            //            std::cout << "X[neighbor]="; print_vec(X[neighbor->ID]); std::cout << std::endl;
            //            std::cout << "ex=" << ex << std::endl;
            //            nx = nx1.size();
            //            std::cout << "rnn=" ; print_vec(nx1) ; std::cout << std::endl;
            //            std::cout << "X[i]="; print_vec(X[i]) ; std::cout << std::endl;

            // debug code, TODO remove
//            if (nx == 0)
//            {
//                nx = 1; // TODO update with true bugfix!!
//                auto dot1 = X[neighbor->ID];
//                auto dot2 = X[i];

//                auto dist_eps_d = std::nextafter(dist, std::numeric_limits<decltype(dist)>::max()); // this is instead of replacing < with <= in Tree // added by Max F in order to match Julia code logic without updating Tree

//                auto rnn_set_test = xTree.rnn(X[i], ex_eps);
//                auto rnn_set_test_2 = xTree.rnn(X[i], ex_eps + 1e-07);
//                auto rnn_set_test_2_2 = xTree.rnn(X[i], ex_eps + 1e-07);
//                auto rnn_set_test_3 = xTree.rnn(X[neighbor->ID], ex_eps);
//                auto rnn_set_test_4 = tree.rnn(XY[neighbor->ID], dist_eps_d);
//                auto rnn_set_test_5 = tree.rnn(XY[i], dist_eps_d);
//                auto knn_res_set_test = tree.knn(XY[i],k+1); // repeats well
//                auto knn_res_set_test_4 = tree.knn(XY[neighbor->ID],k+1); // repeats well

//                std::cout << "zero distance\n";
//            }
        } else {
            throw std::runtime_error("this version not allowed");
        }
        entropyEstimate -= 1.0/N*boost::math::digamma(static_cast<double>(nx));
        // std::cout << "entropyEstimate=" << entropyEstimate << std::endl;
        // std::cout << "dist=" << dist << std::endl;
        // std::cout << "nx=" << nx << std::endl;
        // std::cout << "i=" << i << std::endl;
    }
    return entropyEstimate;
}



template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type // added by Max F
mutualInformation(const std::vector<std::vector<T>> & Xc,
                  const std::vector<std::vector<T>> & Yc, T logbase = 2.0) {
    std::vector<std::vector<T>> XY;
    combine(Xc,Yc,XY);
//    using Cheb = metric::distance::Chebyshev<T>; // replaced by Max F
//    return entropy<T,Cheb>(Xc, 3, logbase, Cheb()) + entropy<T,Cheb>(Yc, 3, logbase, Cheb())
//        - entropy<T, Cheb>(XY, 3, logbase, Cheb());
    return entropy<T>(Xc, logbase) + entropy<T>(Yc, logbase) // entropy overload for integers is not implemented yet
        - entropy<T>(XY, logbase);

}


// template<typename T, typename Metric>
// T mutualInformation(const std::vector<std::vector<T>> & Xc,
//                     const std::vector<std::vector<T>> & Yc,
//                     int k = 3, T logbase=2 ,
//                     Metric metric = Metric(), int version = 2) {
//     T N = Xc.size();
//     T dx = Xc[0].size();
//     T dy = Yc[0].size();

//     auto X = Xc;
//     auto Y = Yc;
//     add_noise(X);
//     auto pe =  pluginEstimator(Y);
//     auto pEstimate = pe.first;
//     auto yVals = pe.second;
//     auto Nxgy =pEstimate*N ;
//     auto eeXgY = boost::math::digamma(k) + boost::math::digamma(Nxgy);
//     auto entropyEstimate = entropy<T,Metric>(X,k,logbase, metric);

//     if (dy>1) {
//         Yp = [data[:,i] for i=1:size(data, 2)];
//         for(std::size_t i = 0; i < pEstimate.size(); i++) {
//             for(std::size_t j = 0; j < Yp.size(); j++) {
//                 tmp = pEstimate[i]*entropy(X[:, Yp[j]==yVals[i]]);
//                 entropyEstimate-=pEstimate[i]*entropy(X[:, Yp[j]==yVals[i]]);
//             }
//         }
//     } else
//         for(std::size_t i = 0; i < pEstimate.size(); i++) {
//             entropyEstimate-=pEstimate[i]*entropy(X[:, reshape(Y.==yVals[i],(:))], k=k, logbase=logbase, treeType=treeType)
//                 }
// }
// return abs(entropyEstimate)
//                     }
#endif
