/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include <iostream>
#include <fstream>

#include <chrono>
#include "../../modules/distance.hpp"


std::tuple<std::vector<std::vector<double>>, std::vector<double>> readData(std::string filename)
{
	std::fstream fin;

	fin.open(filename, std::ios::in);
	
	std::vector<double> row;
	std::string line, word, w;

	std::vector<std::vector<double>> rows;
	std::vector<double> labels;

	// omit headers 
	//getline(fin, line);

	while (getline(fin, line))
	{
		//std::cout << "row " << i << std::endl;
		std::stringstream s(line);
		//std::cout << " -> " << line << std::endl;

		row.clear();
		int i = 0;
		while (getline(s, word, '\t'))
		{
			//std::cout << " --> " << word << std::endl;
			
			if (i >= 2)
			{
				labels.push_back(std::atof(word.c_str()));
			}
			else
			{
				row.push_back(std::atof(word.c_str()));
			}
			i++;
		}

		rows.push_back(row);
	}

	return { rows, labels };
}


int main()
{
	/******************** examples for Kohonen Distance **************************/

	std::cout << "Kohonen Distance example have started" << std::endl;
	std::cout << "" << std::endl;

	int grid_w = 3;
	int grid_h = 2;
	
	using Record = std::vector<double>;
		
	std::vector<Record> train_dataset = {
		{0, 0},
		{1, 0},
		{2, 0},

		{0, 1},
		{1, 1},
		{2, 1},
	};

	double result;


	///
	
	metric::Kohonen<double, std::vector<double>> distance_1(train_dataset, grid_w, grid_h);

	for (int i = 1; i < 6; i++)
	{
		result = distance_1(train_dataset[0], train_dataset[i]);
		std::cout << "result: " << result << std::endl;
	}
	std::cout << "" << std::endl;
	result = distance_1(train_dataset[0], train_dataset[1]);
	assert(result > 0 && result < 2);
	result = distance_1(train_dataset[0], train_dataset[2]);
	assert(result > 1 && result < 3);
	result = distance_1(train_dataset[0], train_dataset[3]);
	assert(result > 0 && result < 2);
	result = distance_1(train_dataset[0], train_dataset[4]);
	assert(result > 1 && result < 3);
	result = distance_1(train_dataset[0], train_dataset[5]);
	assert(result > 2 && result < 4);
	

	///
	
    using Vector = std::vector<double>;
    using Metric = metric::Euclidean<double>;
    using Graph = metric::Grid6; 
    using Distribution = std::uniform_real_distribution<double>; 

	Distribution distr(-1, 1);

    metric::SOM<Vector, Graph, Metric> som_model(Graph(grid_w, grid_h), Metric(), 0.8, 0.2, 50, distr);
	
    if (!som_model.isValid()) {
    	std::cout << "SOM is not valid" << std::endl;
    	return EXIT_FAILURE;
    }

	som_model.train(train_dataset);
	
	metric::Kohonen<double, Vector, Graph, Metric> distance_2(som_model);
	
	for (int i = 1; i < 6; i++)
	{
		result = distance_2(train_dataset[0], train_dataset[i]);
		std::cout << "result: " << result << std::endl;
	}
	std::cout << "" << std::endl;
	result = distance_2(train_dataset[0], train_dataset[1]);
	assert(result > 0 && result < 2);
	result = distance_2(train_dataset[0], train_dataset[2]);
	assert(result > 1 && result < 3);
	result = distance_2(train_dataset[0], train_dataset[3]);
	assert(result > 0 && result < 2);
	result = distance_2(train_dataset[0], train_dataset[4]);
	assert(result > 1 && result < 3);
	result = distance_2(train_dataset[0], train_dataset[5]);
	assert(result > 2 && result < 4);
	

	///
	// negative values
	
    metric::SOM<Vector, Graph, Metric> negative_som_model(Graph(grid_w, grid_h), Metric(), -0.8, -0.2, 50, distr);
	negative_som_model.train(train_dataset);
	
	metric::Kohonen<double, Vector, Graph, Metric> distance_3(negative_som_model);
	
	for (int i = 1; i < 6; i++)
	{
		result = distance_3(train_dataset[0], train_dataset[i]);
		std::cout << "result: " << result << std::endl;
	}
	std::cout << "" << std::endl;
	result = distance_3(train_dataset[0], train_dataset[1]);
	assert(result == 0);
	result = distance_3(train_dataset[0], train_dataset[2]);
	assert(result == 0);
	result = distance_3(train_dataset[0], train_dataset[3]);
	assert(result == 0);
	result = distance_3(train_dataset[0], train_dataset[4]);
	assert(result == 0);
	result = distance_3(train_dataset[0], train_dataset[5]);
	assert(result == 0);

	

	return 0;
}
