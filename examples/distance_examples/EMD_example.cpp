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
#include "../../modules/distance.hpp"
#include <boost/gil/gil_all.hpp> 
#include <boost/gil/extension/io/jpeg_io.hpp>


int main()
{
	/******************** examples for Earth Mover Distance **************************/
	// example for picture


	std::cout << "we have started" << std::endl; 
	std::cout << "" << std::endl;

	boost::gil::rgb8_image_t img1;
	boost::gil::rgb8_image_t img2;
	boost::gil::jpeg_read_image("assets/sample0.jpg_resized.jpg", img1);
	boost::gil::jpeg_read_image("assets/sample1.jpg_resized.jpg", img2);
	std::cout << "Read complete, got an image " << img1.width()
		<< " by " << img1.height() << " pixels\n";
	std::cout << "Read complete, got an image " << img2.width()
		<< " by " << img2.height() << " pixels\n";

	boost::gil::rgb8_pixel_t px = *const_view(img1).at(5, 10);
	std::cout << "The pixel at 5,10 is "
		<< (int)px[0] << ','
		<< (int)px[1] << ','
		<< (int)px[2] << '\n';

	auto gray_img1 = boost::gil::color_converted_view<boost::gil::gray8_pixel_t>(const_view(img1));
	auto gray_img2 = boost::gil::color_converted_view<boost::gil::gray8_pixel_t>(const_view(img2));

	typedef int edm_Type;

	//size_t im1_R = img1.size() / 6;
	//size_t im1_C = img1[0].size() / 6;
	size_t im1_R = img1.width();
	size_t im1_C = img1.height();

	// serialize_mat2vec
	std::vector<edm_Type> i1;
	std::vector<edm_Type> i2;

	std::cout << "iterate through grayscale" << std::endl;
	for (size_t i = 0; i < im1_R * im1_C; ++i)
	{
		i1.push_back(gray_img1[i]);
		i2.push_back(gray_img2[i]);
	}

	typedef int edm_Type;

	//size_t im1_R = img1.size() / 6;
	//size_t im1_C = img1[0].size() / 6;
	//std::cout << "Read complete, got an image " << img1.size()
	//	<< " by " << img1[0].size() << " pixels\n";

	//// serialize_mat2vec
	//std::vector<edm_Type> i1;
	//std::vector<edm_Type> i2;

	//for (size_t i = 0; i < im1_R; ++i)
	//{
	//	for (size_t j = 0; j < im1_C; ++j)
	//	{
	//		i1.push_back(img1[i][j]);
	//		i2.push_back(img2[i][j]);
	//	}
	//}

	auto cost_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<edm_Type>(im1_C, im1_R);
	auto maxCost = metric::EMD_details::max_in_distance_matrix(cost_mat);

	std::cout << "we have started" << std::endl;
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
