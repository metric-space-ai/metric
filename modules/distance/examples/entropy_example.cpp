/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include <random>
#include <cmath>
#include "distance/details/k-related/entropy.hpp"
#include "distance/details/k-related/Standards.hpp"
#include <iostream>

#include "distance/details/k-related/chebyshev.hpp"

//template<typename T>
//struct Chebyshev {
//    T operator()(const std::vector<T> & lhs, const std::vector<T> & rhs) const {
//        T res = 0;
//        for(std::size_t i = 0; i < lhs.size(); i++) {
//            auto m = std::abs(lhs[i]- rhs[i]);
//            if(m > res)
//                res = m;
//        }
//        return res;
//    }
//};


int main() {
    std::vector<std::vector<double>> v = {{1,1}, {2,2}, {3,3}, {5,5}};
//    std::vector<std::vector<double>> v = {{1,1}, {2,2}, {3,3}, {5}};
//    std::vector<std::vector<double>> v = {{1}, {2}, {3}, {5}};

    //auto e = entropy<double,metric::distance::Euclidian<double>>(v);
    auto e = entropy<double, metric::distance::Chebyshev<double>>(v, 3, 2, metric::distance::Chebyshev<double>());
    std::cout << "Chebyshev: " << e << std::endl;

    e = entropy<double, metric::distance::P_norm<double>>(v, 3, 2, metric::distance::P_norm<double>(3));
    std::cout << "General Minkowsky, 3: " << e << std::endl;

    e = entropy<double, metric::distance::P_norm<double>>(v, 3, 2, metric::distance::P_norm<double>(2));
    std::cout << "General Minkowsky, 2: " << e << std::endl;

    e = entropy<double, metric::distance::Euclidian<double>>(v, 3, 2, metric::distance::Euclidian<double>());
    std::cout << "Euclidean: " << e << std::endl;

    e = entropy<double, metric::distance::P_norm<double>>(v, 3, 2, metric::distance::P_norm<double>(1));
    std::cout << "General Minkowsky, 1: " << e << std::endl;

    e = entropy<double, metric::distance::Manhatten<double>>(v, 3, 2, metric::distance::Manhatten<double>());
    std::cout << "Manhatten: " << e << std::endl;


    return 0; // TODO remove

    std::vector<std::vector<double>> v1 = {{5,5}, {2,2}, {3,3}, {5,5}};
    std::vector<std::vector<double>> v2 = {{5,5}, {2,2}, {3,3}, {5,5}};
//    std::vector<std::vector<double>> v1 = {{5,5}, {2,2}, {3,3}, {5}};
//    std::vector<std::vector<double>> v2 = {{5,5}, {2,2}, {3,3}};

//    std::vector<std::vector<double>> v1 = {{5}, {2}, {3}, {5}};
//    std::vector<std::vector<double>> v2 = {{5}, {2}, {3}, {5}};

    auto pe = pluginEstimator(v1);
    for(auto & d : pe.first) {
        std::cout << d << ", ";
    }
    std::cout << std::endl;

    return 0; // TODO remove


    auto e1 = mutualInformation<double, metric::distance::Chebyshev<double>>(v1,v2);
    std::cout<< e1 << std::endl;

    std::cout << mutualInformation<double>(v1, v2) << std::endl;
    return 0;
}
