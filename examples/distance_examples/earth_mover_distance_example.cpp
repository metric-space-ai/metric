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
	    /******************** examples for Earth Mover Distance **************************/
    // example for picture
    std::cout << "EMD example have started" << std::endl;
    std::cout << "" << std::endl;

    typedef int emd_Type;

    size_t im1_R = img1.size() / 6;
    size_t im1_C = img1[0].size() / 6;

    // serialize_mat2vec
    std::vector<emd_Type> i1;
    std::vector<emd_Type> i2;

    for (size_t i = 0; i < im1_R; ++i) {
        for (size_t j = 0; j < im1_C; ++j) {
            i1.push_back(img1[i][j]);
            i2.push_back(img2[i][j]);
        }
    }

    auto ground_distance_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<emd_Type>(im1_C, im1_R);
    auto max_distance = metric::EMD_details::max_in_distance_matrix(ground_distance_mat);

    std::cout << "" << std::endl;

    metric::EMD<emd_Type> distance(ground_distance_mat, max_distance);

    // assumes that i1 and i2 are serialized vectors of the image matrices, and cost_mat contains a distance matrix that takes into account the original pixel locations.
    auto t1 = std::chrono::steady_clock::now();
    auto result = distance(i1, i2);
    auto t2 = std::chrono::steady_clock::now();
    std::cout << "result: " << result
              << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << " s)" << std::endl;
    std::cout << "" << std::endl;

    std::cout << "swap records and calculate again" << std::endl;
    std::cout << "" << std::endl;
	
    t1 = std::chrono::steady_clock::now();
    result = distance(i2, i1);
    t2 = std::chrono::steady_clock::now();
    std::cout << "result: " << result
              << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << " s)" << std::endl;
    std::cout << "" << std::endl;

	//	
	
    ground_distance_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<emd_Type>(3, 3);
    max_distance = metric::EMD_details::max_in_distance_matrix(ground_distance_mat);
    distance = metric::EMD<emd_Type>(ground_distance_mat, max_distance);

	// vector

	std::vector<emd_Type> vector_1 = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<emd_Type> vector_2 = {0, 0, 0, 0, 1, 0, 0, 0, 0};

	t1 = std::chrono::steady_clock::now();
    result = distance(vector_1, vector_2);
    t2 = std::chrono::steady_clock::now();
    std::cout << "result: " << result
              << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << " s)" << std::endl;
    std::cout << "" << std::endl;

	//// array

	//emd_Type array_1[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
 //   emd_Type array_2[] = {0, 0, 0, 0, 1, 0, 0, 0, 0};	

	//t1 = std::chrono::steady_clock::now();
 //   result = distance(array_1, array_2);
 //   t2 = std::chrono::steady_clock::now();
 //   std::cout << "result: " << result
 //             << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
 //             << " s)" << std::endl;
 //   std::cout << "" << std::endl;

	// std::array

	std::array<emd_Type, 9> std_array_1 = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::array<emd_Type, 9> std_array_2 = {0, 0, 0, 0, 1, 0, 0, 0, 0};	

	t1 = std::chrono::steady_clock::now();
    result = distance(std_array_1, std_array_2);
    t2 = std::chrono::steady_clock::now();
    std::cout << "result: " << result
              << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << " s)" << std::endl;
    std::cout << "" << std::endl;

	// deque

	std::deque<emd_Type> deque_1 = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::deque<emd_Type> deque_2 = {0, 0, 0, 0, 1, 0, 0, 0, 0};

	t1 = std::chrono::steady_clock::now();
    result = distance(deque_1, deque_2);
    t2 = std::chrono::steady_clock::now();
    std::cout << "result: " << result
              << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << " s)" << std::endl;
    std::cout << "" << std::endl;

	//// map

	//std::map<int, emd_Type> map_1;
 //   std::map<int, emd_Type> map_2;
	//
	//for (auto i = 0; i < 9; i++)
	//{
	//	map_1[i] = 0;
	//	map_2[i] = 0;
	//}
	//map_2[5] = 1;
	//
	//t1 = std::chrono::steady_clock::now();
 //   result = distance(map_1, map_2);
 //   t2 = std::chrono::steady_clock::now();
 //   std::cout << "result: " << result
 //             << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
 //             << " s)" << std::endl;
 //   std::cout << "" << std::endl;

    return 0;
}
