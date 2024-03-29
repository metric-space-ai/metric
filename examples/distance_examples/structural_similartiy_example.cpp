﻿/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include <iostream>
#include <chrono>
#include "assets/test_data.cpp"
#include "metric/distance.hpp"

int main()
{
	/******************** examples for Structural Similarity (for images) **************************/
	// example for picture

	typedef int edm_Type;

	std::cout << "SSIM example have started" << std::endl;
	std::cout << "" << std::endl;
	
	metric::SSIM<double, std::vector<uint8_t>> distance;

	// assumes that i1 and i2 are serialized vectors of the image matrices.
	auto t1 = std::chrono::steady_clock::now();
	auto result1 = distance(img1, img2);
	auto t2 = std::chrono::steady_clock::now();
	std::cout << "result1: " << result1 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;

	std::cout << "swap records and calculate again" << std::endl;
	std::cout << "" << std::endl;

	auto result2 = distance(img2, img1);
	auto t3 = std::chrono::steady_clock::now();
	std::cout << "result2: " << result2 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;



	return 0;
}
