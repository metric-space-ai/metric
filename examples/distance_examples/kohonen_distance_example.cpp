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

	/* Load data */
	auto [train_dataset, labels] = readData("assets/Compound.txt");
	
	metric::kohonen_distance<double, std::vector<double>> distance_1(train_dataset, 5, 6);

	auto t1 = std::chrono::steady_clock::now();
	auto result = distance_1(train_dataset[0], train_dataset[1]);
	auto t2 = std::chrono::steady_clock::now();
	std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;

	
	
	int grid_w = 6;
	int grid_h = 4;
	
    using Vector = std::vector<double>;
    using Metric = metric::Euclidian<double>;
    using Graph = metric::Grid6; 
    using Distribution = std::uniform_real_distribution<double>; 
	Distribution distr(-1, 1);

	Metric distance;	

    metric::SOM<Vector, Graph, Metric> som_model(Graph(grid_w, grid_h), Metric(), 0.8, 0.2, 20, distr);

    if (!som_model.isValid()) {
    	std::cout << "SOM is not valid" << std::endl;
    	return EXIT_FAILURE;
    }

	som_model.train(train_dataset);
	
	metric::kohonen_distance<double, Vector, Graph, Metric> distance_2(som_model);

	t1 = std::chrono::steady_clock::now();
	result = distance_2(train_dataset[0], train_dataset[1]);
	t2 = std::chrono::steady_clock::now();
	std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;



	return 0;
}
