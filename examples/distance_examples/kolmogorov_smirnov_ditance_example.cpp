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
#include "assets/test_data.cpp"


int main()
{
	    /******************** examples for Kolmogorov-Smirnov Distance **************************/
    // example for picture
    std::cout << "Kolmogorov-Smirnov distance example have started" << std::endl;
    std::cout << "" << std::endl;	

	/*** here are some data records ***/
	std::vector<double> samples_1 = { 0, 1.5, 1, 2, 2, 2, 2, 3, 3, 4 };
	std::vector<double> samples_2 = { 0, 1, 2, 3, 3, 2, 1, 0, 2, 2 };
	std::vector<double> samples_3 = { 0, 0, 0, 3, 3, 2, 1, 0, 2 };
	   
    metric::KolmogorovSmirnov<std::vector<double>, double> distance;

    // assumes that i1 and i2 are serialized vectors of the image matrices, and cost_mat contains a distance matrix that takes into account the original pixel locations.
    auto t1 = std::chrono::steady_clock::now();
    auto result1 = distance(samples_2, samples_3);
    auto t2 = std::chrono::steady_clock::now();
    std::cout << "result1: " << result1
              << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << " s)" << std::endl;
    std::cout << "" << std::endl;

    return 0;
}
