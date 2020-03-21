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
	    /******************** examples for Earth Mover’s Distance **************************/
    // example for picture
    std::cout << "Earth Mover's distance example have started" << std::endl;
    std::cout << "" << std::endl;	

	/*** here are some data records ***/
	std::vector<double> samples_1 = { 0, 1, 2, 3, 3, 2, 1, 0, 2, 2 };
	std::vector<double> samples_2 = { 0, 0, 2, 3, 3, 2, 1, 0, 2, 2 };
	   
    metric::RandomEMD<std::vector<double>, double> distance_1;


	//
 //   typedef int emd_Type;

 //   auto cost_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<emd_Type>(3, 3);
 //   auto maxCost = metric::EMD_details::max_in_distance_matrix(cost_mat);

 //   metric::EMD<emd_Type> distance_orig(cost_mat, maxCost);

	//print_matrix(cost_mat);




    auto t1 = std::chrono::steady_clock::now();
    auto result = distance_1(samples_1, samples_2);
    auto t2 = std::chrono::steady_clock::now();
    std::cout << "result: " << result
              << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << " s)" << std::endl;
    std::cout << "" << std::endl;

	//
	   
    metric::RandomEMD<std::vector<double>, double> distance_2(0.0001);

    t1 = std::chrono::steady_clock::now();
    result = distance_2(samples_1, samples_2);
    t2 = std::chrono::steady_clock::now();
    std::cout << "result: " << result
              << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << " s)" << std::endl;
    std::cout << "" << std::endl;

	//
	   
    metric::RandomEMD<std::vector<double>, double> distance_3(0.1);

    t1 = std::chrono::steady_clock::now();
    result = distance_3(samples_1, samples_2);
    t2 = std::chrono::steady_clock::now();
    std::cout << "result: " << result
              << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << " s)" << std::endl;
    std::cout << "" << std::endl;

    return 0;
}
