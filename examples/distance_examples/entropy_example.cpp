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
#include <chrono>


int main() {


    // Entropy

    std::cout << "Entropies:" << std::endl;

    {
        std::vector<std::vector<double>> v = { {5,5}, {2,2}, {3,3}, {5,1} };
        auto ekpn = metric::Entropy<metric::Euclidean<std::vector<double>>>(metric::Euclidean<std::vector<double>>(), 2, 3);
        std::cout << "entropy_kpN, Euclidean, STL: " << ekpn(v)
                  << " | " << ekpn.estimate(v, 2)
                  << std::endl;
        auto e = metric::EntropySimple<metric::Euclidean<std::vector<double>>>(metric::Euclidean<std::vector<double>>(), 2);
        std::cout << "entropy_simple, Euclidean, STL: " << e(v)
                  << " | " << e.estimate(v, 2)
                  << std::endl;
    }
    {
        using Record = blaze::DynamicVector<double>;
        std::vector<Record> v = { {5,5}, {2,2}, {3,3}, {5,1} };
        auto ekpn = metric::Entropy<metric::Euclidean<Record>>(metric::Euclidean<Record>(), 2, 3);
        std::cout << "entropy_kpN, Euclidean, Blaze: " << ekpn(v)
                  << " | " << ekpn.estimate(v, 2)
                  << std::endl;
        auto e = metric::Entropy<metric::Euclidean<Record>>(metric::Euclidean<Record>(), 2);
        std::cout << "entropy_simple, Euclidean, Blaze: " << e(v)
                  << " | " << e.estimate(v, 2)
                  << std::endl;
    }
    {
        using Record = Eigen::Array<double, 1, Eigen::Dynamic>;
        std::vector<std::vector<double>> v_stl = { {5,5}, {2,2}, {3,3}, {5,1} }; // this is only to initialize Eigen vector
        std::vector<Record> v;
        for (size_t i = 0; i < v_stl.size(); ++i)
            v.push_back(Record::Map(v_stl[i].data(), v_stl[i].size()));
        auto ekpn = metric::Entropy<metric::Euclidean<Record>>(metric::Euclidean<Record>(), 2, 3);
        std::cout << "entropy_kpN, Euclidean, Eigen: " << ekpn(v)
                  << " | " << ekpn.estimate(v, 2)
                  << std::endl;
        auto e = metric::Entropy<metric::Euclidean<Record>>(metric::Euclidean<Record>(), 2);
        std::cout << "entropy_simple, Euclidean, Blaze: " << e(v)
                  << " | " << e.estimate(v, 2)
                  << std::endl;
    }
#ifdef ARMA_EXISTS
    {
        using Record = arma::Row<double>;
        std::vector<Record> v = { {5,5}, {2,2}, {3,3}, {5,1} };
        auto ekpn = metric::Entropy<metric::Euclidean<Record>>(metric::Euclidean<Record>(), 2, 3);
        std::cout << "entropy_kpN, Euclidean, Armadillo: " << ekpn(v)
                  << " | " << ekpn.estimate(v, 2)
                  << std::endl;
        auto e = metric::Entropy<metric::Euclidean<Record>>(metric::Euclidean<Record>(), 2);
        std::cout << "entropy_simple, Euclidean, Blaze: " << e(v)
                  << " | " << e.estimate(v, 2)
                  << std::endl;
    }
#endif

    std::cout << std::endl;

    std::cout << "\n\nEntropy of uniformly distributed r. v.s:\n";

    {
        using Record = std::vector<double>;
        std::mt19937 gen(1);
        std::uniform_real_distribution<double> dis(0.0, 1.0);

        std::vector<Record> v;
        for (size_t i = 0; i<1000; ++i) {
            v.push_back({dis(gen), dis(gen)});
        }

        auto ekpn = metric::Entropy<metric::Euclidean<Record>>(metric::Euclidean<Record>(), 2, 3);
        auto startTime = std::chrono::steady_clock::now();
        auto result1 = ekpn(v);
        auto endTime = std::chrono::steady_clock::now();
        auto time1 = double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000;
        startTime = std::chrono::steady_clock::now();
        auto result2 = ekpn.estimate(v);
        endTime = std::chrono::steady_clock::now();
        auto time2 = double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000;
        std::cout << "entropy_kpN, Euclidean, STL, 1000 elements: "
                  << result1 << " (" << time1 << "ns) | "
                  << result2 << " (" << time2 << "ns) "
                  << std::endl;
    }
    {
        using Record = blaze::DynamicVector<double>;
        std::mt19937 gen(1);
        std::uniform_real_distribution<double> dis(0.0, 1.0);

        std::vector<Record> v;
        for (size_t i = 0; i<1000; ++i) {
            v.push_back({dis(gen), dis(gen)});
        }

        auto ekpn = metric::Entropy<metric::Euclidean<Record>>(metric::Euclidean<Record>(), 2, 3);
        std::cout << "entropy_kpN, Euclidean, Blaze, 1000 elements: " << ekpn(v)
                  << " | " << ekpn.estimate(v)
                  << std::endl;
    }
    {
        using Record = Eigen::Array<double, 1, Eigen::Dynamic>;;
        std::mt19937 gen(1);
        std::uniform_real_distribution<double> dis(0.0, 1.0);

        std::vector<std::vector<double>> v_stl;
        for (size_t i = 0; i<1000; ++i) {
            v_stl.push_back({dis(gen), dis(gen)});
        }
        std::vector<Record> v;
        for (size_t i = 0; i < v_stl.size(); ++i)
            v.push_back(Record::Map(v_stl[i].data(), v_stl[i].size()));
        auto ekpn = metric::Entropy<metric::Euclidean<Record>>(metric::Euclidean<Record>(), 2, 3);
        std::cout << "entropy_kpN, Euclidean, Eigen, 1000 elements: " << ekpn(v)
                  << " | " << ekpn.estimate(v)
                  << std::endl;
    }
#ifdef ARMA_EXISTS
    {
        using Record = arma::Row<double>;
        std::mt19937 gen(1);
        std::uniform_real_distribution<double> dis(0.0, 1.0);

        std::vector<Record> v;
        for (size_t i = 0; i<1000; ++i) {
            v.push_back({dis(gen), dis(gen)});
        }
        auto ekpn = metric::Entropy<metric::Euclidean<Record>>(metric::Euclidean<Record>(), 2, 3);
        std::cout << "entropy_kpN, Euclidean, Armadillo, 1000 elements: " << ekpn(v)
                  << " | " << ekpn.estimate(v)
                  << std::endl;
    }
#endif

    return 0;
}
//*/
