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
#include "../../modules/space.hpp"
#include <boost/gil/gil_all.hpp> 
#include <boost/gil/extension/io/jpeg_io.hpp>
//#include <boost/gil.hpp> 
//#include <boost/gil/extension/io/jpeg.hpp>
#include <boost/gil/extension/numeric/sampler.hpp>
#include <boost/gil/extension/numeric/resample.hpp>

template <typename T>
void saveToCsv(std::string filename, const std::vector<std::vector<T>> &mat, const std::vector<std::string> &features)
{
	std::ofstream outputFile;

	// create and open the .csv file
	outputFile.open(filename);

	// write the file headers
	outputFile << ",";
	for (auto i = 0; i < features.size(); ++i)
	{
		outputFile << features[i];
		outputFile << ",";
	}
	//outputFile << "date";
	outputFile << std::endl;

	// last item in the mat is date
	for (auto i = 0; i < mat.size(); ++i)
	{
		outputFile << features[i] << ",";
		//outputFile << dates[i] << ",";
		for (auto j = 0; j < mat[i].size(); j++)
		{
			outputFile << mat[i][j] << ",";
		}
		outputFile << std::endl;
	}

	// close the output file
	outputFile.close();
}

int main()
{
	/******************** examples for Earth Mover Distance **************************/
	// example for picture


	std::cout << "we have started" << std::endl; 
	std::cout << "" << std::endl;

	int num_images = 20;
	typedef int edm_Type;

	std::vector<std::string> names(num_images);
	std::vector < boost::gil::rgb8_image_t > jpegs(num_images);
	std::vector < std::vector < edm_Type > > grayJpegs(num_images);
	for (size_t i = 0; i < num_images; ++i)
	{
		names[i] = "sample_" + std::to_string(i);
		//boost::gil::read_image("assets/sample" + std::to_string(i) + ".jpg_resized.jpg", jpegs[i], boost::gil::jpeg_tag());
		boost::gil::jpeg_read_image("assets/sample" + std::to_string(i) + ".jpg_resized.jpg", jpegs[i]);

		boost::gil::rgb8_image_t squareScaled(50, 50);
		boost::gil::resize_view(const_view(jpegs[i]), view(squareScaled), boost::gil::bilinear_sampler());

		//auto gray = boost::gil::color_converted_view<boost::gil::gray8_pixel_t>(const_view(jpegs[i]));
		auto gray = boost::gil::color_converted_view<boost::gil::gray8_pixel_t>(const_view(squareScaled));
		//for (size_t j = 0; j < 1000; ++j)
		for (size_t j = 0; j < gray.size(); ++j)
		{
			grayJpegs[i].push_back(gray[j]);
		}
	}
	std::cout << "Read complete, got an image " << jpegs[0].width()
		<< " by " << jpegs[0].height() << " pixels\n";
	std::cout << "And grayscaled and size scaled image ready " << grayJpegs[0].size() << " items\n";

	boost::gil::rgb8_pixel_t px = *const_view(jpegs[0]).at(5, 10);
	std::cout << "The pixel at 5,10 is "
		<< (int)px[0] << ','
		<< (int)px[1] << ','
		<< (int)px[2] << '\n';
	

	//size_t im1_R = img1.size() / 6;
	//size_t im1_C = img1[0].size() / 6;
	size_t im1_R = jpegs[0].width();
	size_t im1_C = jpegs[0].height();

	// serialize_mat2vec
	//std::vector<edm_Type> i1;
	//std::vector<edm_Type> i2;

	std::cout << "iterate through grayscale" << std::endl;
	for (size_t i = 0; i < im1_R * im1_C; ++i)
	{
		//i1.push_back(gray_img1[i]);
		//i2.push_back(gray_img2[i]);
	}

	//typedef int edm_Type;

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



	/*** initialize the tree ***/
	metric::Tree<std::vector < edm_Type >, metric::EMD<edm_Type>> cTree;
	std::cout << "tree created" << std::endl;

	/*** add data records ***/

	for (size_t i = 0; i < grayJpegs.size(); ++i)
	{
		if (i == 5 || i == 10)
		{
			cTree.print();
		}
		auto t1 = std::chrono::steady_clock::now();
		std::cout << "insert " << i << std::endl;
		cTree.insert(grayJpegs[i]);
		auto t2 = std::chrono::steady_clock::now();
		std::cout << "inserted " << i << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;

	}
	cTree.print();


	//std::vector < std::vector < edm_Type > > distance_matrix(num_images, std::vector < edm_Type >(num_images));

	//std::cout << "create distance matrix" << std::endl;
	//auto cost_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<edm_Type>(im1_C, im1_R);
	//std::cout << "create max distance matrix" << std::endl;
	//auto maxCost = metric::EMD_details::max_in_distance_matrix(cost_mat);

	//std::cout << "we have started" << std::endl;
	//std::cout << "" << std::endl;

	//metric::EMD<edm_Type> distance(cost_mat, maxCost);

	//for (size_t i = 0; i < num_images; ++i)
	//{
	//	for (size_t j = i + 1; j < num_images; ++j)
	//	{
	//		// assumes that i1 and i2 are serialized vectors of the image matrices, and cost_mat contains a distance matrix that takes into account the original pixel locations.
	//		auto t1 = std::chrono::steady_clock::now();
	//		auto result1 = distance(grayJpegs[i], grayJpegs[j]);
	//		auto t2 = std::chrono::steady_clock::now();
	//		std::cout << "result for " << i << " <-> " << j << " is:" << result1 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	//		std::cout << "" << std::endl;

	//		distance_matrix[i][j] = result1;
	//		distance_matrix[j][i] = result1;
	//	}
	//}

	//saveToCsv("distances.csv", distance_matrix, names);

	/*std::cout << "swap records and calculate again" << std::endl;
	std::cout << "" << std::endl;

	auto result2 = distance(i2, i1);
	auto t3 = std::chrono::steady_clock::now();
	std::cout << "result2: " << result2 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;*/
	
	return 0;
}
