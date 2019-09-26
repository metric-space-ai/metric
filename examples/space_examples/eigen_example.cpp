/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Panda Team
*/

#include <vector>
#include <iostream>
#include "../../3rdparty/Eigen/Dense"
#include "../../modules/space.hpp"

using recType = Eigen::VectorXd;

template <typename T>
struct recMetric {
    T operator()(const Eigen::VectorXd &p, const Eigen::VectorXd &q) const
    {
        return (p - q).lpNorm<1>(); // L1 norm
    }
};

int main()
{
	std::cout << "Eigen space example have started" << std::endl;
	std::cout << "" << std::endl;


	recType x(8);
	
	std::vector<recType> table;

	x << 0, 1, 1, 1, 1, 1, 2, 3;
	table.push_back(x);
	x << 1, 1, 1, 1, 1, 2, 3, 4;
	table.push_back(x);
	x << 2, 2, 2, 1, 1, 2, 0, 0;
	table.push_back(x);
	x << 3, 3, 2, 2, 1, 1, 0, 0;
	table.push_back(x);
	x << 4, 3, 2, 1, 0, 0, 0, 0;
	table.push_back(x);
	x << 5, 3, 2, 1, 0, 0, 0, 0;
	table.push_back(x);
	x << 4, 6, 2, 2, 1, 1, 0, 0;
	table.push_back(x);
		
    metric::Tree<recType, recMetric<double>> cTree(table);

	//

	std::vector<recType> table2;
	
	x << 3, 7, 2, 1, 0, 0, 0, 0;
	table2.push_back(x);
	x << 2, 8, 2, 1, 0, 0, 0, 0;
	table2.push_back(x);

    cTree.insert(table2);

    cTree.print();

    return 0;
}
