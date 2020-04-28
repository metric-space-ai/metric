/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include "../../modules/distance.hpp"
#include "../../modules/correlation/entropy.hpp"

#include <iostream>
#include <vector>
#include <random>



int main() {


    // Entropy

    std::cout << "Entropies:" << std::endl;

    std::vector<std::vector<double>> v = { {5,5}, {2,2}, {3,3}, {5,1} };

    {
        auto ekpn = metric::Entropy<metric::Euclidean<std::vector<double>>>(metric::Euclidean<std::vector<double>>(), 2, 3);
        auto e = ekpn(v);
        std::cout << "entropy_kpN, using Chebyshev: " << e << std::endl;
    }

    std::cout << std::endl;




    return 0;
}
//*/
