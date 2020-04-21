/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include <iostream>
#include <chrono>

#include "../../modules/distance.hpp"


int main()
{
	std::cout << "Standart Distances example have started" << std::endl;
	std::cout << "" << std::endl;

	/*** here are some data records ***/
	std::vector<double> v0 = { 0, 1, 1, 1, 1, 1, 2, 3 };
	std::vector<double> v1 = { 1, 1, 1, 1, 1, 2, 3, 4 };


	/******************** examples for Euclidean (L2) Metric **************************/
    std::cout << "Euclidean (L2) Metric" << std::endl;
    metric::Euclidean<std::vector<double>> EuclideanL2Distance;
    auto startTime_1 = std::chrono::steady_clock::now();
    auto result_1 = EuclideanL2Distance(v0, v1);
    auto endTime_1 = std::chrono::steady_clock::now();
    std::cout << "result: " << result_1 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime_1 - startTime_1).count()) / 1000 << " ms)" << std::endl;
    std::cout << "" << std::endl;
    // out:
    // Euclidean (L2) Metric
    // result: 2 (Time = 0.018 ms)



	return 0;
}
