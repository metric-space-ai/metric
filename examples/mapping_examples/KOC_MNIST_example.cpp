/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#if defined(__linux__)
	#include <dirent.h>
#endif

#include <vector>
#include <any>

#include <iostream>
#include <fstream>

#if defined(_WIN64)
#include <filesystem>
#endif

#include <chrono>

#include "../../modules/utils/ThreadPool.cpp"
#include "../../modules/utils/Semaphore.h"

#include "assets/json.hpp"
#include "../../modules/mapping.hpp"


using json = nlohmann::json;


template <typename T>
void matrix_print(const std::vector<std::vector<T>> &mat)
{

    std::cout << "[";
    std::cout << std::endl;
	for (int i = 0; i < mat.size(); i++)
	{
		std::cout << "  [ ";
		if (mat[i].size() > 0)
		{
			for (int j = 0; j < mat[i].size() - 1; j++)
			{
				std::cout << mat[i][j] << ", ";
			}
			std::cout << mat[i][mat[i].size() - 1];
		}
			
		std::cout << " ]" << std::endl;
    }
    std::cout << std::endl;
    std::cout << "]" << std::endl;
}

template <typename T>
void vector_print(const std::vector<T> &vec)
{

    std::cout << "[ ";
    for (int i = 0; i < vec.size() - 1; i++)
    {
        std::cout << vec[i] << ", ";
    }
    std::cout << vec[vec.size() - 1] << " ]" << std::endl;
}

template <typename T>
void vector_print(const std::vector<T> &vec,const size_t width, const size_t height)
{
	if ((width * height) != vec.size()) {
		std::cout << "width * height != vector.size()" << std::endl;
		return;
	}
	
	int max_digits = 1;
	for (auto index = 0; index < vec.size(); ++index) {
		int pos = 10;
		int digits_num = 1;
		while (vec[index] / pos >= 1)
		{
			digits_num++;
			pos *= 10;
		}
		if (digits_num > max_digits)
		{
			max_digits = digits_num;
		}
	}

	for (auto index = 0; index < vec.size(); ++index) {
		
		int pos = 10;
		int digits_num = 1;
		while (vec[index] / pos >= 1)
		{
			digits_num++;
			pos *= 10;
		}
		for (auto i = 0; i < max_digits - digits_num; ++i) {
			std::cout << " ";
		}
		std::cout << vec[index] << " ";

		if ((index + 1) % width == 0) {
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;
}

template <typename T>
void image_print(const std::vector<T> &vec, const size_t width, const size_t height)
{
	if ((width * height) != vec.size()) {
		std::cout << "width * height != vector.size()" << std::endl;
		return;
	}
	
	int max_digits = 3;

	for (auto index = 0; index < vec.size(); ++index) {
		
		int pos = 10;
		int digits_num = 1;
		if (vec[index] >= 10)
		{
			digits_num++;
		}
		if (vec[index] >= 100)
		{
			digits_num++;
		}
		for (auto i = 0; i < max_digits - digits_num; ++i) {
			std::cout << " ";
		}
		std::cout << static_cast<unsigned int>(vec[index]) << " ";

		if ((index + 1) % width == 0) {
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;
}

std::tuple<std::vector<std::vector<double>>, std::vector<int>> readCsvData(std::string filename, char delimeter)
{
	std::fstream fin;

	fin.open(filename, std::ios::in);
	
	std::vector<double> row;
	std::string line, word, w;

	std::vector<std::vector<double>> rows;
	std::vector<int> labels;

	// omit header
	getline(fin, line);

	int i = 0;
	while (getline(fin, line))
	{
		i++;
		std::stringstream s(line);

		// get label
		getline(s, word, delimeter);
		labels.push_back(std::stoi(word));

		row.clear();
		while (getline(s, word, delimeter))
		{			
			row.push_back(std::stod(word));
		}

		rows.push_back(row);
	}

	return { rows, labels };
}

std::tuple<std::vector<std::string>, std::vector<std::vector<double>>> readCsvData2(std::string filename, char delimeter)
{
	std::fstream fin;

	fin.open(filename, std::ios::in);
	
	std::vector<std::string> dates;
	std::vector<double> row;
	std::string line, word, w;

	std::vector<std::vector<double>> rows;
	
	int day, month, year, hour, minute, second;
	int added_days = 0;
	bool was_yesterday = false;

	// omit headers 
	getline(fin, line);

	int i = 0;
	while (getline(fin, line))
	{
		i++;
		std::stringstream s(line);

		getline(s, word, delimeter);
		sscanf(word.c_str(), "%4d-%2d-%2d %2d:%2d:%2d", &year, &month, &day, &hour, &minute, &second);
		if (was_yesterday && hour * 60 + minute >= 4 * 60)
		{
			dates.push_back(word);
			rows.push_back(row);
			row.clear();
		}

		if (hour * 60 + minute < 4 * 60)
		{
			was_yesterday = true;
		}
		else
		{
			was_yesterday = false;
		}

		while (getline(s, word, delimeter))
		{			
			row.push_back(std::stod(word));
		}

	}

	// erase first element with partial data
	rows.erase(rows.begin());
	dates.erase(dates.end() - 1);

	return { dates, rows };
}

std::vector<double> generate_image(size_t size, size_t min, size_t max)
{	
	std::vector<double> result;
		
	auto random_seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine random_generator(random_seed);
    std::exponential_distribution<double> generator(3.5);
	
    for (int i = 0; i < size; ++i) {
        result.push_back(static_cast<int>(generator(random_generator) * (max - min) + min));
    }

	return result;
}

std::vector<double> noise_image(std::vector<double> image, int min, int max)
{	
	std::vector<double> result;
		
	auto random_seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine random_generator(random_seed);	
    std::uniform_int_distribution<int> generator(min, max);

    for (int i = 0; i < image.size(); ++i) {
        image[i] += generator(random_generator);
		if (image[i] > 255)
		{
			image[i] = 255;
		}
    }

	return image;
}

///

int main(int argc, char *argv[])
{
	std::cout << "KOC for MNIST example have started" << std::endl;
	std::cout << std::endl;

	using Record = std::vector<double>;
				
	size_t best_w_grid_size = 4;
	size_t best_h_grid_size = 3;

	// if overrided from arguments
	
	if (argc > 3)
	{
		best_w_grid_size = std::stod(argv[2]);
		best_h_grid_size = std::stod(argv[3]);
	}
	
	std::vector<Record> dataset;
	std::vector<int> labels;
	std::vector<Record> test_set;
	std::vector<int> test_labels;
	
	std::tie(dataset, labels) = readCsvData("assets/mnist100.csv", ',');
	
	std::cout << std::endl;
	std::cout << "labels:" << std::endl;
	vector_print(labels);
	   
	std::tie(test_set, test_labels) = readCsvData("assets/MNIST_anomalies.csv", ',');
	test_set.push_back(dataset[0]);
	test_set.push_back(dataset[1]);
	test_set.push_back(dataset[4]);
	test_set.push_back(generate_image(28 * 28, 0, 255));
	test_set.push_back(generate_image(28 * 28, 0, 255));
	test_set.push_back(generate_image(28 * 28, 0, 1));
	test_set.push_back(noise_image(dataset[0], 0, 150));
	test_set.push_back(noise_image(dataset[1], 0, 150));

	///
	//

	int num_clusters = 10;

	// random seed for repeateable results
	long long random_seed = 777;

	// extra deviation of the clusters from original in the KOC
	double sigma = 1.75;

	metric::KOC_factory<Record, metric::Grid4, metric::CosineInverted<double>> simple_koc_factory(best_w_grid_size, best_h_grid_size, sigma, 0.8, 0.0, 200, 0, 255, 4, 2.0, random_seed);    
	auto simple_koc = simple_koc_factory(dataset, num_clusters); 
	
	
	std::cout << std::endl;
	std::cout << "train dataset:" << std::endl;

	auto anomalies = simple_koc.check_if_anomaly(dataset);	
	std::cout << std::endl;
	std::cout << "anomalies:" << std::endl;
	vector_print(anomalies);
	
	auto assignments = simple_koc.result(dataset);	
	std::cout << std::endl;
	std::cout << "assignments:" << std::endl;
	vector_print(assignments);


	// accuracy
	
	std::vector<std::vector<int>> clusters(11);	
	for (auto i = 0; i < assignments.size(); ++i)
	{
		clusters[assignments[i]].push_back(labels[i]);
	}
	
	std::vector<int> assignment_to_label(11, -1);
	for (auto i = 1; i < clusters.size(); ++i)
	{
		std::vector<int> labels_in_cluster(10);
		for (auto lbl : clusters[i])
		{
			labels_in_cluster[lbl]++;
		}		
		std::vector<int>::iterator result = std::max_element(labels_in_cluster.begin(), labels_in_cluster.end());
		assignment_to_label[i] = std::distance(labels_in_cluster.begin(), result);	
	}
	
	int num_matched = 0;
	for (auto i = 0; i < assignments.size(); ++i)
	{
		if (assignment_to_label[assignments[i]] == labels[i])
		{
			num_matched++;
		}
	}
	std::cout << std::endl;
	std::cout << "num_matched: " << num_matched << std::endl;
	std::cout << "accuracy: " << (double)num_matched / assignments.size() << std::endl;


	// test dataset	
	
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "test dataset:" << std::endl;

	anomalies = simple_koc.check_if_anomaly(test_set);	
	std::cout << std::endl;
	std::cout << "anomalies:" << std::endl;
	vector_print(anomalies);
	
	assignments = simple_koc.result(test_set);	
	std::cout << std::endl;
	std::cout << "assignments:" << std::endl;
	vector_print(assignments);

	///
	//
	//image_print(test_set[0], 28, 28);	
	//image_print(test_set[1], 28, 28);	
	//image_print(test_set[2], 28, 28);	
	//image_print(test_set[3], 28, 28);	
	//image_print(test_set[4], 28, 28);	
	//image_print(test_set[5], 28, 28);	
	//image_print(test_set[6], 28, 28);	
	//image_print(test_set[7], 28, 28);	

	//metric::CosineInverted<double> distance;
	//
	//std::cout << std::endl;
	//std::cout << distance(test_set[0], test_set[1]) << std::endl; // 4 and 3
	//std::cout << distance(test_set[0], test_set[2]) << std::endl; // 4 and 4
	//std::cout << std::endl;
	//std::cout << distance(test_set[0], test_set[3]) << std::endl; // 4 and noise(0..255)
	//std::cout << distance(test_set[0], test_set[4]) << std::endl; // 4 and noise(0..255)
	//std::cout << distance(test_set[0], test_set[5]) << std::endl; // 4 and noise(0..1)
	//std::cout << std::endl;
	//std::cout << distance(test_set[0], test_set[6]) << std::endl; // 4 and 4 (noised)
	//std::cout << distance(test_set[0], test_set[7]) << std::endl; // 4 and 3 (noised)
	//std::cout << std::endl;
	//std::cout << distance(test_set[3], test_set[4]) << std::endl; // noise(0..255) and noise(0..255)
	//std::cout << distance(test_set[3], test_set[5]) << std::endl; // noise(0..255) and noise(0..1)


    return 0;
}
