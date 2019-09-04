/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Panda Team
*/

#include <vector>
#include <iostream>
#include <Eigen/Dense>
#include "../../modules/space.hpp"
//#include "../../3rdparty/eigen/Core/Matrix.h"

using recType = Eigen::VectorXd;

template <typename T>
struct recMetric {
    T operator()(const Eigen::VectorXd &p, const Eigen::VectorXd &q) const
    {
        return (p - q).cwise().abs().sum(); // L1 norm
    }
};

int main()
{
	std::cout << "Eigen space example have started" << std::endl;
	std::cout << "" << std::endl;

    /*** batch insert ***/
    std::vector<std::vector<double>> table = {
        { 0, 1, 1, 1, 1, 1, 2, 3 },
        { 1, 1, 1, 1, 1, 2, 3, 4 },
        { 2, 2, 2, 1, 1, 2, 0, 0 },
        { 3, 3, 2, 2, 1, 1, 0, 0 },
        { 4, 3, 2, 1, 0, 0, 0, 0 },
        { 5, 3, 2, 1, 0, 0, 0, 0 },
        { 4, 6, 2, 2, 1, 1, 0, 0 },
    };
	
    metric::Tree<recType, recMetric> cTree(table);

    std::vector<std::vector<double>> table2 = { { 3, 7, 2, 1, 0, 0, 0, 0 }, { 2, 8, 2, 1, 0, 0, 0, 0 } };

    cTree.insert(table2);

    cTree.print();

    return 0;
}
