/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <chrono>
#include "../../modules/distance.hpp"



bool is_inside_rect(std::vector<double> &rect, std::vector<double> &point)
{
	if (point[0] >= rect[0] && point[0] <= rect[0] + rect[2])
	{
		if (point[1] >= rect[1] && point[1] <= rect[1] + rect[3])
		{
			return true;
		}
	}

	return false;
}


template <typename Record>
void saveToCsv(std::string filename, const std::vector<Record> &mat, const std::vector<std::string> &features)
{
	std::ofstream outputFile;

	// create and open the .csv file
	outputFile.open(filename);

	// write the file headers
	for (auto i = 0; i < features.size(); ++i)
	{
		outputFile << features[i];
		outputFile << ",";
	}
	outputFile << std::endl;

	// last item in the mat is date
	for (auto i = 0; i < mat.size(); ++i)
	{
		//outputFile << dates[i] << ";";
		for (auto j = 0; j < mat[i].size(); j++)
		{
			outputFile << mat[i][j] << ",";
		}
		outputFile << std::endl;
	}

	// close the output file
	outputFile.close();
}

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


int main()
{
	/******************** examples for Kohonen Distance **************************/

	std::cout << "Kohonen Distance example have started" << std::endl;
	std::cout << "" << std::endl;

	int grid_w = 5;
	int grid_h = 5;

	metric::Euclidean<double> euclidean_distance;
	
	using Record = std::vector<double>;

	std::vector<Record> data;

	int num_points = 1000;

	for (size_t i = 0; i < num_points; i++)
	{
		data.push_back({ double (std::rand() % 100), double (std::rand() % 100) });
	}
	
	std::vector<double> rect_1 = { 20, 20, 80, 20 };
	std::vector<double> rect_2 = { 0, 60, 80, 20 };

	std::vector<double> zero_point = { 0, 100 };
	std::vector<double> opposite_zero_point = { 100, 0 };
	
	std::vector<double> min_point = opposite_zero_point;
	std::vector<double> max_point = zero_point;

	
	std::vector<Record> filtered_data;
	
	for (size_t i = 0; i < num_points; i++)
	{
		if (!is_inside_rect(rect_1, data[i]) && !is_inside_rect(rect_2, data[i]))
		{
			filtered_data.push_back(data[i]);

			if (euclidean_distance(zero_point, data[i]) < euclidean_distance(zero_point, min_point))
			{
				min_point = data[i];
			}
			

			if (euclidean_distance(opposite_zero_point, data[i]) < euclidean_distance(opposite_zero_point, max_point))
			{
				max_point = data[i];
			}
		}
	}

	saveToCsv("data.csv", filtered_data, {"X", "Y"});
	
	metric::Kohonen<double, std::vector<double>> distance_1(filtered_data, grid_w, grid_h);

	auto t1 = std::chrono::steady_clock::now();
	auto result = distance_1(min_point, max_point);
	auto t2 = std::chrono::steady_clock::now();
	std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;

	auto bmu_1 = distance_1.som_model.BMU(min_point);
	auto bmu_2 = distance_1.som_model.BMU(max_point);
	
	distance_1.print_shortest_path(bmu_1, bmu_2);
    std::cout << std::endl;
	auto path = distance_1.get_shortest_path(bmu_1, bmu_2);

	double result_for_check = 0;
	
	std::vector<Record> som_nodes = distance_1.som_model.get_weights();
	metric::Euclidean<double> eu_distance;
	std::vector<std::vector<int>> matrix_path;
	auto prev_node = min_point;
	for (size_t i = 0; i < path.size(); i++)
	{
		matrix_path.push_back({path[i]});
		result_for_check += eu_distance(prev_node, som_nodes[path[i]]);
		prev_node = som_nodes[path[i]];
	}
	result_for_check += eu_distance(prev_node, max_point);
	std::cout << "result test: " << result_for_check << std::endl;

	
	std::cout << "result: " << distance_1(zero_point, min_point) << std::endl;
	std::cout << "result test: " << eu_distance(zero_point, min_point) << std::endl;

	saveToCsv("som.csv", som_nodes, {"X", "Y"});

	saveToCsv("path.csv", matrix_path, {"node"});
	
	std::cout << distance_1.som_model.BMU(min_point) << std::endl;
	std::cout << distance_1.som_model.BMU(max_point) << std::endl;


	return 0;
}
