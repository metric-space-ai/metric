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
#include <boost/gil.hpp> 
#include <boost/gil/extension/io/jpeg.hpp>
#include <boost/gil/extension/numeric/sampler.hpp>
#include <boost/gil/extension/numeric/resample.hpp>


int main()
{
	/******************** examples for Earth Mover Distance **************************/
	// example for picture


	std::cout << "we have started" << std::endl; 
	std::cout << "" << std::endl;

	typedef int edm_Type;
	std::vector<edm_Type> i1;
	std::vector<edm_Type> i2;

	boost::gil::rgb8_image_t jpeg1;
	boost::gil::rgb8_image_t jpeg2;

	boost::gil::read_image("assets/sample0.jpg_resized.jpg", jpeg1, boost::gil::jpeg_tag());
	boost::gil::read_image("assets/sample1.jpg_resized.jpg", jpeg2, boost::gil::jpeg_tag());
	auto gray1 = boost::gil::color_converted_view<boost::gil::gray8_pixel_t>(const_view(jpeg1));
	auto gray2 = boost::gil::color_converted_view<boost::gil::gray8_pixel_t>(const_view(jpeg2));
	for (size_t i = 0; i < gray1.size(); ++i)
	{
		i1.push_back(gray1[i]);
	}
	for (size_t i = 0; i < gray2.size(); ++i)
	{
		i2.push_back(gray2[i]);
	}

	size_t im1_R = jpeg1.width();
	size_t im1_C = jpeg1.height();


	std::cout << "create distance matrix" << std::endl;
	auto cost_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<edm_Type>(im1_C, im1_R);
	std::cout << "create max distance matrix" << std::endl;
	auto maxCost = metric::EMD_details::max_in_distance_matrix(cost_mat);

	std::cout << "" << std::endl;

	metric::EMD<edm_Type> distance(cost_mat, maxCost);


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
