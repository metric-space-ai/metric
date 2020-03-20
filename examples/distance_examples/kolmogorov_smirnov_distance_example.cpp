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
	std::vector<double> samples_1 = { 0, 1, 2, 3, 3, 2, 1, 0, 2, 2 };
	std::vector<double> samples_2 = { 0, 0, 2, 3, 3, 2, 1, 0, 2, 2 };
	//std::vector<double> samples_1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	//std::vector<double> samples_2 = { 1, 2, 3, 4, 5 };
	//   
    metric::KolmogorovSmirnov<std::vector<double>, double> distance;

    auto t1 = std::chrono::steady_clock::now();
    auto result1 = distance(samples_1, samples_2);
    auto t2 = std::chrono::steady_clock::now();
    std::cout << "result: " << result1
              << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << " s)" << std::endl;
    std::cout << "" << std::endl;

    return 0;
}
