/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include <iostream>
#include <chrono>
#include "assets/test_data.cpp"
#include "modules/distance.hpp"

int main()
{
	/******************** examples for Structural Similartiy (for images) **************************/
	// example for picture

	typedef int edm_Type;

	size_t im1_R = img1.size() / 6;
	size_t im1_C = img1[0].size() / 6;

	// serialize_mat2vec
	std::vector<edm_Type> i1;
	std::vector<edm_Type> i2;

	for (size_t i = 0; i < im1_R; ++i)
	{
		for (size_t j = 0; j < im1_C; ++j)
		{
			i1.push_back(img1[i][j]);
			i2.push_back(img2[i][j]);
		}
	}


	std::cout << "we have started" << std::endl;
	std::cout << "" << std::endl;
	
	metric::SSIM<std::vector<edm_Type>> distance;

	// assumes that i1 and i2 are serialized vectors of the image matrices, and cost_mat contains a distance matrix that takes into account the original pixel locations.
	auto t1 = std::chrono::steady_clock::now();
	auto result1 = distance(i1, i2);
	auto t2 = std::chrono::steady_clock::now();
	std::cout << "result1: " << result1 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;

	std::cout << "swap records and calculate again" << std::endl;
	std::cout << "" << std::endl;

	auto result2 = distance(i2, i1);
	auto t3 = std::chrono::steady_clock::now();
	std::cout << "result2: " << result2 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;



	return 0;
}
