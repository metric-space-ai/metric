#include <vector>
#include <random>
#include <cmath>

// for digamma function
#include <boost/math/special_functions/digamma.hpp>

//#include <boost/container_hash/hash.hpp> // replaced with <boost/functional/hash.hpp>, TODO check
#include <boost/functional/hash.hpp>

//#include "3dparty/metric_space/details/tree.hpp"
#include "space/details/tree.hpp"
//#include "3dparty/metric_distance/details/Standards.hpp"
#include "distance/details/k-related/Standards.hpp"
//#include "3dparty/metric_distance/details/chebyshev.hpp"
#include "chebyshev.hpp"

template<typename T>
void add_noise(std::vector<std::vector<T>> & data) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
    double c = 1e-10;
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
T entropy(std::vector<std::vector<T>> data, std::size_t k = 3, T logbase = 2, Metric metric = metric::distance::Euclidian<T>()) {
    if(data.empty() || data[0].empty()) {
        return 0;
    }
    double p = 1;
    if constexpr ( std::is_same<Metric, typename metric::distance::Euclidian<T>>::value) {
            p = 2;
        } else if constexpr (std::is_same<Metric, typename metric::distance::P_norm<T>>::value) {
            p = metric.p;
        }

    double N = data.size();
    double d = data[0].size();
    add_noise(data);
    metric_space::Tree<std::vector<T>, Metric> tree(data,-1, metric);
    double cb = d * log(logbase,2.0) + d*log(logbase, std::tgamma(1+1/p)) - log(logbase, std::tgamma(1+d/p));
    double entropyEstimate = boost::math::digamma(N) - boost::math::digamma(k)
        + cb + d * log(logbase, 2.0);
    for(std::size_t i = 0; i < N; i++) {
        auto res = tree.knn(data[i],k+1);
        // print(res);
        // std::cout << "entropyEstimate=" << entropyEstimate << std::endl;
        // std::cout << "last_dist=" << res.back().second << std::endl;

        entropyEstimate += d/N*log(logbase,res.back().second);
    }
    return entropyEstimate;
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
std::pair<std::vector<T>,std::vector<std::vector<T>>>
pluginEstimator(const std::vector<std::vector<T>> & Y) {
    std::vector<std::vector<T>> uniqueVal = unique(Y);
    std::vector<T> counts(uniqueVal.size());
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
    T N = X.size();
    T dx = X[0].size();
    T dy = Y[0].size();
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
template<typename T, typename Metric>
T mutualInformation(const std::vector<std::vector<T>> & Xc,
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
    std::cout << "entropyEstimate1=" << entropyEstimate << std::endl;
    if(version == 2) {
        entropyEstimate -= 1/static_cast<double>(k);
    }
    std::cout << "entropyEstimate2=" << entropyEstimate << std::endl;
    metric_space::Tree<std::vector<T>, Metric> xTree(X,-1,metric);
    metric_space::Tree<std::vector<T>, Metric> yTree(Y,-1,metric);
    for(std::size_t i = 0; i < N; i++) {
        auto res = tree.knn(XY[i],k+1);
        auto neighbor = res.back().first;
        auto dist = res.back().second;
        std::size_t nx = 0;
        if(version == 1) {
            nx = xTree.rnn(X[i],dist).size();
        } else if(version == 2) {
            auto ex = metric(X[neighbor->ID],X[i]);
            nx = xTree.rnn(X[i],ex).size();
            //            std::cout << "X[neighbor]="; print_vec(X[neighbor->ID]); std::cout << std::endl;
            //            std::cout << "ex=" << ex << std::endl;
            //            nx = nx1.size();
            //            std::cout << "rnn=" ; print_vec(nx1) ; std::cout << std::endl;
            //            std::cout << "X[i]="; print_vec(X[i]) ; std::cout << std::endl;
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
T mutualInformation(const std::vector<std::vector<T>> & Xc,
                    const std::vector<std::vector<T>> & Yc, T logbase = 2.0) {
    std::vector<std::vector<T>> XY;
    combine(Xc,Yc,XY);
    using Cheb = metric::distance::Chebyshev<T>;
    return entropy<T,Cheb>(Xc, 3, logbase, Cheb()) + entropy<T,Cheb>(Yc, 3, logbase, Cheb())
        - entropy<T, Cheb>(XY, 3, logbase, Cheb());
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
