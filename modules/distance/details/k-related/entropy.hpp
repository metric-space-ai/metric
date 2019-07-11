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
typename std::enable_if<!std::is_integral<T>::value, T>::type  // replaced T with conditional type by Max F
entropy(std::vector<std::vector<T>> data, std::size_t k = 3, T logbase = 2,
        Metric metric = metric::distance::Euclidian<T>());

// overload for integer types // added by Max F
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
entropy(std::vector<std::vector<T>> data, T logbase = 2);



template<typename T>
std::pair<std::vector<double>,std::vector<std::vector<T>>> // T replaced with double by Max F
pluginEstimator(const std::vector<std::vector<T>> & Y);



template<typename T, typename Metric = metric::distance::Chebyshev<T>>
typename std::enable_if<!std::is_integral<T>::value, T>::type // line added by Max F
mutualInformation(const std::vector<std::vector<T>> & Xc,
                  const std::vector<std::vector<T>> & Yc,
                  int k = 3,  Metric metric = Metric(), int version = 2);


template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type // line added by Max F
mutualInformation(const std::vector<std::vector<T>> & Xc,
                  const std::vector<std::vector<T>> & Yc, T logbase = 2.0);
    }
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




template<typename T>
typename std::enable_if<!std::is_integral<T>::value, T>::type
variationOfInformation(const std::vector<std::vector<T>> & Xc,
                  const std::vector<std::vector<T>> & Yc, int k = 3, T logbase = 2.0)
{
    using Cheb = metric::distance::Chebyshev<T>;
    return entropy<T,Cheb>(Xc, k, logbase, Cheb()) + entropy<T,Cheb>(Yc, k, logbase, Cheb())
         - 2 * mutualInformation<T>(Xc, Yc, k);
}


template<typename T>
typename std::enable_if<!std::is_integral<T>::value, T>::type
variationOfInformation_normalized(const std::vector<std::vector<T>> & Xc,
                  const std::vector<std::vector<T>> & Yc, int k = 3, T logbase = 2.0)
{
    using Cheb = metric::distance::Chebyshev<T>;
    auto mi = mutualInformation<T>(Xc, Yc, k);
    return 1 - ( mi / (entropy<T,Cheb>(Xc, k, logbase, Cheb()) + entropy<T,Cheb>(Yc, k, logbase, Cheb()) - mi) );
}




#endif
