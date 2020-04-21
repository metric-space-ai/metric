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
#include "../../3rdparty/blaze/Blaze.h"

int main()
{
	std::cout << "Standart Distances example have started" << std::endl;
	std::cout << "" << std::endl;

	/*** here are some data records ***/
    std::vector<double> stlv0 = { 0, 1, 1, 1, 1, 1, 2, 3 };
    std::vector<double> stlv1 = { 1, 1, 1, 1, 1, 2, 3, 4 };
    blaze::DynamicVector<double> blazev0 = { 0, 1, 1, 1, 1, 1, 2, 3 };
    blaze::DynamicVector<double> blazev1 = { 1, 1, 1, 1, 1, 2, 3, 4 };



	/******************** examples for Euclidean (L2) Metric **************************/
    {
        std::cout << "Euclidean (L2) Metric in STL vectors" << std::endl;
        metric::Euclidean<std::vector<double>> EuclideanL2Distance;
        auto startTime = std::chrono::steady_clock::now();
        auto result = EuclideanL2Distance(stlv0, stlv1);
        auto endTime = std::chrono::steady_clock::now();
        std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000 << " ms)" << std::endl;
        std::cout << "" << std::endl;
        // out:
        // Euclidean (L2) Metric
        // result: 2 (Time = 0.018 ms)
    }
    {
        std::cout << "Euclidean (L2) Metric in Blaze vectors" << std::endl;
        metric::Euclidean<blaze::DynamicVector<double>> EuclideanL2Distance;
        auto startTime = std::chrono::steady_clock::now();
        auto result = EuclideanL2Distance(blazev0, blazev1);
        auto endTime = std::chrono::steady_clock::now();
        std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000 << " ms)" << std::endl;
        std::cout << "" << std::endl;
        // out:
        // Euclidean (L2) Metric
        // result: 2 (Time = 0.006 ms)
    }


	return 0;
}
