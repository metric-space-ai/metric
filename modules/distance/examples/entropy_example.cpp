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

#include <random> // for random vector generation

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
    std::vector<std::vector<double>> v = {{5,5}, {2,2}, {3,3}, {5,5}};
//    std::vector<std::vector<double>> v = {{1,1}, {2,2}, {3,3}, {5}};
//    std::vector<std::vector<double>> v = {{1}, {2}, {3}, {5}};
//    std::vector<std::vector<double>> v = {{1}, {2}, {3}, {2}};

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


    std::vector<std::vector<double>> v1 = {{5,5}, {2,2}, {3,3}, {5,5}};
    std::vector<std::vector<double>> v2 = {{5,5}, {2,2}, {3,3}, {1,1}};
//    std::vector<std::vector<double>> v1 = {{5,5}, {2,2}, {3,3}, {5}};
//    std::vector<std::vector<double>> v2 = {{5,5}, {2,2}, {3,3}};

//    std::vector<std::vector<double>> v1 = {{5}, {2}, {3}, {5}};
//    std::vector<std::vector<double>> v2 = {{5}, {2}, {3}, {5}};

//    auto pe = pluginEstimator(v1);

    std::vector<std::vector<int>> v_int = {{5,5}, {2,2}, {3,3}, {5,5}};
    auto pe = pluginEstimator(v_int);
    for(auto & d : pe.first) {
        std::cout << d << ", ";
    }
    std::cout << std::endl;


    std::cout << "\n";

    auto e1 = mutualInformation<double>(v1,v1);
    std::cout<< e1 << std::endl << std::endl;

    std::cout << mutualInformation<double>(v1, v1) << std::endl;
    std::cout<< mutualInformation<double, metric::distance::Chebyshev<double>>(v1, v1, 3, metric::distance::Chebyshev<double>(), 1) << std::endl;
    std::cout << mutualInformation<double>(v2, v2) << std::endl;
    std::cout<< mutualInformation<double, metric::distance::Chebyshev<double>>(v2, v2, 3, metric::distance::Chebyshev<double>(), 1) << std::endl;
    std::cout << mutualInformation<double>(v1, v2) << std::endl;
    std::cout<< mutualInformation<double, metric::distance::Chebyshev<double>>(v1, v2, 3, metric::distance::Chebyshev<double>(), 1) << std::endl;


//    std::cout << "\n";

//    std::cout << "comparing distributions of results for simple input vector\n";
//    for (size_t i = 0; i<30; i++)
//        std::cout << entropy<double, metric::distance::Chebyshev<double>>(v, 3, 2, metric::distance::Chebyshev<double>()) << "\n";


    std::cout << "\n";

    //std::vector<std::vector<int>> i1 = {{5,5}, {2,2}, {3,3}, {5,5}};
    //std::vector<std::vector<int>> i2 = {{5,5}, {2,2}, {3,3}, {1,1}};
    //std::cout << mutualInformation<int>(i1, i2) << std::endl; // now this overload only raises the logic_error exception with "Not implemented" message


    std::cout << "random array entropy test\n";

    std::vector<long double> cases1{1,3,2,4,2};
    std::vector<long double> cases2{4,3,2,1,4};

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(0, 4);

    std::vector<std::vector<long double>> rnd;
    for (size_t i = 0; i < 5000; i++)
        rnd.push_back({cases1[uni(rng)], cases2[uni(rng)]});


    for (size_t i = 0; i < 10; i++)
        std::cout << entropy<long double, metric::distance::Chebyshev<long double>>(rnd, 3, 2, metric::distance::Chebyshev<long double>()) << std::endl;
    //std::cout << entropy<double, metric::distance::Chebyshev<double>>(rnd, 3, 2, metric::distance::Chebyshev<double>()) << std::endl;


    std::cout << "random array mutualInformation test\n";

    std::vector<std::vector<long double>> rnd2; // second independent vector with same distribution
    for (size_t i = 0; i < 5000; i++)
        rnd2.push_back({cases1[uni(rng)], cases2[uni(rng)]});

    for (size_t i = 0; i<10; i++)
        std::cout << mutualInformation<long double>(rnd, rnd2) << std::endl;



    // TODO remove

//    std::cout << std::endl;

//    std::normal_distribution<double> dis(0, 1);
//    for (size_t i = 0; i < 10; i++)
//        std::cout << dis(rng) << "\n";

//    std::cout << std::endl;

//    for (size_t i = 0; i < 100; i++)
//        std::cout << cases1[uni(rng)] << " ";



    return 0;
}
