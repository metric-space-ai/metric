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
#include "metric/distance.hpp"


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


std::vector<std::vector<double>> readCsvData(std::string filename, char delimeter = ',')
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
		//getline(s, word, delimeter);
		//labels.push_back(std::stoi(word));

		row.clear();
		while (getline(s, word, delimeter))
		{			
			row.push_back(std::stod(word));
		}

		rows.push_back(row);
	}

	return rows;
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


int main()
{
	/******************** examples for Kohonen Distance **************************/

	std::cout << "Kohonen Distance example have started" << std::endl;
	std::cout << "" << std::endl;

	int grid_w = 3;
	int grid_h = 2;
	
	using Record = std::vector<double>;
		
	std::vector<Record> simple_grid = {
		{0, 0},
		{1, 0},
		{2, 0},

		{0, 1},
		{1, 1},
		{2, 1},
	};
	
	metric::Kohonen<double, std::vector<double>> distance_1(simple_grid, grid_w, grid_h);

	auto t1 = std::chrono::steady_clock::now();
	auto result = distance_1(simple_grid[2], simple_grid[3]);
	auto t2 = std::chrono::steady_clock::now();
	std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;

	//
	
	/* Load data */
	auto [train_dataset, labels] = readData("assets/Compound.txt");	
	
	metric::Kohonen<double, std::vector<double>> distance_2(train_dataset, grid_w, grid_h);

	t1 = std::chrono::steady_clock::now();
	result = distance_2(train_dataset[0], train_dataset[1]);
	t2 = std::chrono::steady_clock::now();
	std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;

	//
	
    using Vector = std::vector<double>;
    using Metric = metric::Euclidean<double>;
    using Graph = metric::Grid6; 
    using Distribution = std::uniform_real_distribution<double>; 

	Distribution distr(-1, 1);

    metric::SOM<Vector, Graph, Metric> som_model(Graph(grid_w, grid_h), Metric(), 0.8, 0.2, 20, distr);
	
    if (!som_model.isValid()) {
    	std::cout << "SOM is not valid" << std::endl;
    	return EXIT_FAILURE;
    }

	som_model.train(train_dataset);
	
	metric::Kohonen<double, Vector, Graph, Metric> distance_3(som_model, train_dataset);

	t1 = std::chrono::steady_clock::now();
	result = distance_3(train_dataset[0], train_dataset[1]);
	t2 = std::chrono::steady_clock::now();
	std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;
	
	// distortion


	double start_learn_rate = 1.0;
	double finish_learn_rate = 0.2;
	size_t iterations = 300;
	double neighborhood_start_size = 10;
	double neighborhood_range_decay = 10;
	long long random_seed = std::chrono::system_clock::now().time_since_epoch().count();

	Distribution distr_4(0, 5);

    metric::SOM<Record, Graph, Metric> som_model_5(Graph(grid_w, grid_h), Metric(), start_learn_rate, finish_learn_rate, iterations, distr_4, neighborhood_start_size, neighborhood_range_decay, random_seed);
	som_model_5.train(simple_grid);
	metric::Kohonen<double, Record, Graph, Metric> distance_5(som_model_5, simple_grid);

	t1 = std::chrono::steady_clock::now();
	result = distance_5.distortion_estimate(simple_grid);
	t2 = std::chrono::steady_clock::now();
	std::cout << "distortion estimate result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;


	
    metric::SOM<Record, Graph, Metric> som_model_4(Graph(grid_w, grid_h), Metric(), start_learn_rate, finish_learn_rate, iterations, distr_4, neighborhood_start_size, neighborhood_range_decay, random_seed);
		
	std::vector<Record> linear_dataset = {
		{0, 0},
		{1, 0},
		{2, 0},

		{3, 0},
		{4, 0},
		{5, 0},
	};
	som_model_4.train(linear_dataset);
	metric::Kohonen<double, Record, Graph, Metric> distance_4(som_model_4, linear_dataset);

	t1 = std::chrono::steady_clock::now();
	result = distance_4.distortion_estimate(linear_dataset);
	t2 = std::chrono::steady_clock::now();
	std::cout << "distortion estimate result for linear dataset: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;



	///////////////////////////////////////////////////////////////////////////////////////////

	std::vector<std::string> dataset_names;
	dataset_names.push_back("BOD2");
	dataset_names.push_back("Chloride");
	dataset_names.push_back("Coal");
	dataset_names.push_back("Ethyl");
	dataset_names.push_back("Isom");
	dataset_names.push_back("Leaves");
	dataset_names.push_back("Lipo");
	dataset_names.push_back("Lubricant");
	//dataset_names.push_back("Nitren");
	dataset_names.push_back("Nitrite");
	dataset_names.push_back("O.xylene");
	dataset_names.push_back("Oilshale");
	dataset_names.push_back("PCB");
	dataset_names.push_back("Pinene");
	//dataset_names.push_back("Pinene2");
	dataset_names.push_back("Rumford");
	dataset_names.push_back("Sacch2");
	dataset_names.push_back("Saccharin");
	
	for (size_t i = 0; i < dataset_names.size(); i++)
	{
		std::vector<Record> r_dataset = readCsvData("assets/" + dataset_names[i] + ".csv");

		metric::Kohonen<double, Record, Graph, Metric> r_distance(r_dataset, grid_w, grid_h);

		result = r_distance.distortion_estimate(r_dataset);
		std::cout << "distortion estimate result for " << dataset_names[i] << ": " << result << std::endl;
		std::cout << "" << std::endl;
	}


	return 0;
}
