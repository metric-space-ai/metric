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
#include "../../modules/space.hpp"
#include <boost/gil.hpp> 
#include <boost/gil/extension/io/jpeg.hpp>


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

	/*** initialize the tree ***/
	std::cout << "tree for pictures been creating" << std::endl;
	metric::Tree<std::vector<edm_Type>, metric::EMD<edm_Type>> cTree;
	std::cout << "inserting picture #1" << std::endl;
	cTree.insert(i1);
	std::cout << "inserting picture #2" << std::endl;
	cTree.insert(i2);
	std::cout << "tree for pictures has created" << std::endl;
	cTree.print();
	
	return 0;
}
