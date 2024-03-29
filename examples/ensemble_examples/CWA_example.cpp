/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include <iostream>

#include "metric/mapping/ensembles/DT/correlation_weighted_accuracy.hpp"

int main()
{
    std::cout << "Correlation Weighted Accuracy example have started" << std::endl;
    std::cout << '\n';

    std::vector<int> g1 = {3, 2, 2, 3, 1, 1}; // Known groups
    std::vector<int> g2 = {4, 2, 2, 2, 1, 1}; // Predicted groups

    double cwa = metric::correlation_weighted_accuracy(g1, g2);

    std::cout << "CWA: " << cwa << std::endl;

    return 0;
}
