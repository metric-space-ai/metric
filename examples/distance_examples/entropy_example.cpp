/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include "../../modules/distance.hpp"
#include "../../modules/correlation/entropy.hpp"

#include "../../3rdparty/blaze/Blaze.h"
#include "../../3rdparty/Eigen/Dense"
#if __has_include(<armadillo>)
    #include <armadillo>
    #define ARMA_EXISTS
#endif // linker parameter: -larmadillo

#include <iostream>
#include <vector>
#include <random>



int main() {


    // Entropy

    std::cout << "Entropies:" << std::endl;

    {
        std::vector<std::vector<double>> v = { {5,5}, {2,2}, {3,3}, {5,1} };
        auto ekpn = metric::Entropy<metric::Euclidean<std::vector<double>>>(metric::Euclidean<std::vector<double>>(), 2, 3);
        auto e = ekpn(v);
        std::cout << "entropy_kpN, using Euclidean, STL: " << e << std::endl;
    }
    {
        using Record = blaze::DynamicVector<double>;
        std::vector<Record> v = { {5,5}, {2,2}, {3,3}, {5,1} };
        auto ekpn = metric::Entropy<metric::Euclidean<Record>>(metric::Euclidean<Record>(), 2, 3);
        auto e = ekpn(v);
        std::cout << "entropy_kpN, using Euclidean, Blaze: " << e << std::endl;
    }
    {
        using Record = Eigen::Array<double, 1, Eigen::Dynamic>;
        std::vector<std::vector<double>> v_stl = { {5,5}, {2,2}, {3,3}, {5,1} }; // this is only to initialize Eigen vector
        std::vector<Record> v;
        for (size_t i = 0; i < v_stl.size(); ++i)
            v.push_back(Record::Map(v_stl[i].data(), v_stl[i].size()));
        auto ekpn = metric::Entropy<metric::Euclidean<Record>>(metric::Euclidean<Record>(), 2, 3);
        auto e = ekpn(v);
        std::cout << "entropy_kpN, using Euclidean, Eigen: " << e << std::endl;
    }
#ifdef ARMA_EXISTS
    {
        using Record = arma::Row<double>;
        std::vector<Record> v = { {5,5}, {2,2}, {3,3}, {5,1} };
        auto ekpn = metric::Entropy<metric::Euclidean<Record>>(metric::Euclidean<Record>(), 2, 3);
        auto e = ekpn(v);
        std::cout << "entropy_kpN, using Euclidean, Armadillo: " << e << std::endl;
    }
#endif

    std::cout << std::endl;




    return 0;
}
//*/
