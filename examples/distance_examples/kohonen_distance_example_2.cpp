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
#include "../../modules/mapping/Redif.hpp"



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

	metric::Euclidean<double> euclidean_distance;
	
	using Record = std::vector<double>;

	std::vector<Record> data;

	int num_points = 1000;

	for (size_t i = 0; i < num_points; i++)
	{
		data.push_back({ double (std::rand() % 100), double (std::rand() % 100) });
	}
	
	std::vector<double> rect_1 = { 30, 20, 70, 20 };
	std::vector<double> rect_2 = { 0, 60, 70, 20 };

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

	//

    using Metric = metric::Euclidean<double>;
    using Graph = metric::Grid8; 
    using Distribution = std::uniform_real_distribution<double>; 
	
	int grid_w = 40;
	int grid_h = 3; 
	
	//double start_learn_rate = 1.0;
	//double finish_learn_rate = 0.3;
	//size_t iterations = 300;
	//double neighborhood_start_size = 100;
	//double neighborhood_range_decay = 100;
	double start_learn_rate = 1.0;
	double finish_learn_rate = 0.0;
	size_t iterations = 1000;
	double neighborhood_start_size = 40;
	double neighborhood_range_decay = 40;
	//long long random_seed = std::chrono::system_clock::now().time_since_epoch().count();
	long long random_seed = 15918678724411133;
	std::cout << "random_seed: " << random_seed << std::endl;

	Distribution distr(0, 100);

    metric::SOM<Record, Graph, Metric> som_model(Graph(grid_w, grid_h), Metric(), start_learn_rate, finish_learn_rate, iterations, distr, neighborhood_start_size, neighborhood_range_decay, random_seed);

	som_model.train(filtered_data);

	// basic Kohonen distance
	
	metric::Kohonen<double, Record, Graph, Metric> distance_1(som_model, filtered_data);

	auto result = distance_1(min_point, max_point);
	std::cout << "result: " << result << std::endl;

	//
	
	std::vector<Record> som_nodes = distance_1.som_model.get_weights();
	saveToCsv("basic_som.csv", som_nodes, {"X", "Y"});
	
	//

	auto bmu_1 = distance_1.som_model.BMU(min_point);
	auto bmu_2 = distance_1.som_model.BMU(max_point);	
	auto path = distance_1.get_shortest_path(bmu_1, bmu_2);

	std::vector<std::vector<int>> matrix_path;
	for (size_t i = 0; i < path.size(); i++)
	{
		matrix_path.push_back({path[i]});
	}
	saveToCsv("basic_path.csv", matrix_path, {"node"});

	//

	std::vector<std::vector<int>> edges;
	auto matrix = distance_1.som_model.get_graph().get_matrix();
	for (int i = 0; i < matrix.rows(); ++i)
	{
		for (int j = i + 1; j < matrix.columns(); ++j) 
		{
			if (matrix(i, j) > 0)
			{
				edges.push_back({i, j});
			}
		}
	}
	saveToCsv("basic_som_edges.csv", edges, {"start", "end"});

	// sparced Kohonen distance
	
	metric::Kohonen<double, Record, Graph, Metric> distance_2(som_model, filtered_data, true, 0.75);

	result = distance_2(min_point, max_point);
	std::cout << "result: " << result << std::endl;

	//
	
	som_nodes = distance_2.som_model.get_weights();
	saveToCsv("sparced_som.csv", som_nodes, {"X", "Y"});
	
	//

	bmu_1 = distance_2.som_model.BMU(min_point);
	bmu_2 = distance_2.som_model.BMU(max_point);	
	path = distance_2.get_shortest_path(bmu_1, bmu_2);

	matrix_path.clear();
	for (size_t i = 0; i < path.size(); i++)
	{
		matrix_path.push_back({path[i]});
	}
	saveToCsv("sparced_path.csv", matrix_path, {"node"});

	//

	edges.clear();
	matrix = distance_2.som_model.get_graph().get_matrix();
	for (int i = 0; i < matrix.rows(); ++i)
	{
		for (int j = i + 1; j < matrix.columns(); ++j) 
		{
			if (matrix(i, j) > 0)
			{
				edges.push_back({i, j});
			}
		}
	}
	saveToCsv("sparced_som_edges.csv", edges, {"start", "end"});


	// Reverse Diffused Kohonen distance

	metric::Kohonen<double, Record, Graph, Metric> distance_3(som_model, filtered_data, false, 0.0, true, 6);

	result = distance_3(min_point, max_point);
	std::cout << "result: " << result << std::endl;

	//
	
	som_nodes = distance_3.som_model.get_weights();
	saveToCsv("rev_diff_som.csv", som_nodes, {"X", "Y"});
	
	//

	bmu_1 = distance_3.som_model.BMU(min_point);
	bmu_2 = distance_3.som_model.BMU(max_point);	
	path = distance_3.get_shortest_path(bmu_1, bmu_2);

	matrix_path.clear();
	for (size_t i = 0; i < path.size(); i++)
	{
		matrix_path.push_back({path[i]});
	}
	saveToCsv("rev_diff_path.csv", matrix_path, {"node"});

	//

	edges.clear();
	matrix = distance_3.som_model.get_graph().get_matrix();
	for (int i = 0; i < matrix.rows(); ++i)
	{
		for (int j = i + 1; j < matrix.columns(); ++j) 
		{
			if (matrix(i, j) > 0)
			{
				edges.push_back({i, j});
			}
		}
	}
	saveToCsv("rev_diff_som_edges.csv", edges, {"start", "end"}); 


	// Reverse Diffused ad Sparced Kohonen distance

	metric::Kohonen<double, Record, Graph, Metric> distance_4(som_model, filtered_data, true, 0.75, true, 6);

	result = distance_4(min_point, max_point);
	std::cout << "result: " << result << std::endl;

	//
	
	som_nodes = distance_4.som_model.get_weights();
	saveToCsv("rev_diff_and_sparced_som.csv", som_nodes, {"X", "Y"});
	
	//

	bmu_1 = distance_4.som_model.BMU(min_point);
	bmu_2 = distance_4.som_model.BMU(max_point);	
	path = distance_4.get_shortest_path(bmu_1, bmu_2);

	matrix_path.clear();
	for (size_t i = 0; i < path.size(); i++)
	{
		matrix_path.push_back({path[i]});
	}
	saveToCsv("rev_diff_and_sparced_path.csv", matrix_path, {"node"});

	//

	edges.clear();
	matrix = distance_4.som_model.get_graph().get_matrix();
	for (int i = 0; i < matrix.rows(); ++i)
	{
		for (int j = i + 1; j < matrix.columns(); ++j) 
		{
			if (matrix(i, j) > 0)
			{
				edges.push_back({i, j});
			}
		}
	}
	saveToCsv("rev_diff_and_sparced_som_edges.csv", edges, {"start", "end"});

	/////////
	

	return 0;
}
