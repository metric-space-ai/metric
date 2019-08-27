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

#include "../../modules/utils/ThreadPool.cpp"
#include "../../modules/utils/Semaphore.h"

#include "../../assets/json.hpp"
#include "../../modules/mapping.hpp"


using json = nlohmann::json;


template <typename T>
void matrix_print(const std::vector<std::vector<T>> &mat)
{

    std::cout << "[";
    for (int i = 0; i < mat.size(); i++)
    {
        for (int j = 0; j < mat[i].size() - 1; j++)
        {
            std::cout << mat[i][j] << ", ";
        }
        std::cout << mat[i][mat[i].size() - 1] << " ]" << std::endl;
        ;
    }
    std::cout << std::endl;
}

template <typename T>
void vector_print(const std::vector<T> &vec)
{

    std::cout << "[";
    for (int i = 0; i < vec.size(); i++)
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

	for (auto index = 0; index < vec.size(); ++index) {
		std::cout << vec[index] << " ";

		if ((index + 1) % width == 0) {
			std::cout << std::endl;
		}
	}
}

void printDataInfo(const json& data)
{
	for (const auto& [key, value]: data.items()) {
		std::cout << key << " " << value.size() << std::endl;
	}
}


std::vector<std::vector<double>> readData(std::string filename)
{
	std::fstream fin;

	fin.open(filename, std::ios::in);
	
	std::vector<double> row;
	std::string line, word, w;

	std::vector<std::vector<double>> rows;

	int i = 0;
	while (getline(fin, line))
	{
		i++;
		//std::cout << "row " << i << std::endl;
		std::stringstream s(line);
		char asciiChar = 9;
		getline(s, word, asciiChar);
		//std::cout << " time: " << word << std::endl;

		row.clear();
		while (getline(s, word, asciiChar))
		{
			//std::cout << " -> " << word << std::endl;
			row.push_back(std::stod(word));
		}

		rows.push_back(row);
	}

	return rows;
}


std::mutex mu;

template <typename T, typename Metric, typename Graph, typename Distribution>
double runConfiguration(int i, std::vector<std::vector<T>> data, Metric distance, Graph graph, Distribution distribution, 
	unsigned int iterations, double start_learn_rate, double final_learn_rate, double neighborhoodSize, double neigbour_range_decay, long long random_seed)
{
	
	mu.lock();
	std::cout << "configuration #" << i << " started" << std::endl;
	std::cout << "  Graph: " << typeid(graph).name() << std::endl;
	std::cout << "  Distance: " << typeid(distance).name() << std::endl;
	std::cout << "  Distribution: " << typeid(distribution).name() << std::endl;
	mu.unlock();

	auto t1 = std::chrono::steady_clock::now();

	metric::SOM<std::vector<T>, Metric, Graph, Distribution> DR(distance, graph, distribution, neighborhoodSize, neigbour_range_decay, random_seed);
	

	/* Train */
	DR.train(data, iterations, start_learn_rate, final_learn_rate);

	double total_distances = 0;
	for (size_t i = 0; i < data.size(); i++)
	{
		auto dimR = DR.reduce(data[i]);
		auto bmu = DR.BMU(data[i]);
		total_distances += dimR[bmu];
	}
		
	auto t2 = std::chrono::steady_clock::now();
	mu.lock();
	std::cout << "configuration #" << i << std::endl;
	std::cout << "  Graph: " << typeid(graph).name() << std::endl;
	std::cout << "  Distance: " << typeid(distance).name() << std::endl;
	std::cout << "  Distribution: " << typeid(distribution).name() << std::endl;
	std::cout << "Total distances: " << total_distances << 
		" mean distance: " << total_distances / data.size() << 
		" (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl << std::endl;
	mu.unlock();

	return total_distances;
}


int main()
{
	std::cout << "SOM example have started" << std::endl;
	std::cout << '\n';

	/* Load data */
	auto speeds = readData("assets/energies_speed_190820.log");


	unsigned concurentThreadsSupported = std::thread::hardware_concurrency();
	std::cout << "Num cores: " << concurentThreadsSupported << std::endl;
	ThreadPool pool(concurentThreadsSupported);
	Semaphore sem;

	//
	
	std::vector<int> graph_types = {0, 1, 2, 3, 4, 5};
	std::vector<int> metric_types = {0, 1, 2, 3, 4, 5};
	std::vector<int> distribution_types = {0, 2, 3};
	
	std::vector<size_t> grid_sizes = {4, 6, 9, 12, 16, 25, 30, 36, 42, 49};
	std::vector<double> s_learn_rates = {0.2, 0.5, 0.8, 1, 1.2};
	std::vector<double> f_learn_rates = {0.2, 0.5, 0.7, 0.9};
	std::vector<double> initial_neighbour_sizes = {0.2, 0.5, 0.7, 0.9};
	std::vector<double> neigbour_range_decays = {0.1, 0.2, 0.3, 0.5};
	std::vector<long long> random_seeds = {0, 100, 10000, 10000000};
	std::vector<unsigned int> iterations_all = {100, 1000, 10000, 100000};
				
	//size_t grid_size = 25;

	//double s_learn_rate = 0.9;
	//double f_learn_rate = 0.4;
				
	//double initial_neighbour_size = 1.2;

	//double neigbour_range_decay = 0;

	//long long random_seed = 0;

	//batch_training

	//unsigned int iterations = 1000;

	int epochs = 1;
	
	std::vector<std::string> graph_type_names = {"Grid4", "Grid6", "Grid8", "Paley", "LPS", "Margulis"};
	std::vector<std::string> metric_type_names = {"Euclidian", "Manhatten", "P_norm", "Euclidian_thresholded", "Cosine", "Chebyshev"};
	std::vector<std::string> distribution_type_names = {"uniform_real_distribution", "normal_distribution", "exponential_distribution"};

	//
	const int count = graph_types.size() * metric_types.size() * distribution_types.size() * 
		grid_sizes.size() * s_learn_rates.size() * f_learn_rates.size() * initial_neighbour_sizes.size() * neigbour_range_decays.size() * random_seeds.size() * iterations_all.size();

	std::vector<double> results(count, INFINITY);

	int i = 0;
	for (auto grid_size : grid_sizes)
	{
		for (auto s_learn_rate : s_learn_rates)
		{
			for (auto f_learn_rate : f_learn_rates)
			{
				for (auto initial_neighbour_size : initial_neighbour_sizes)
				{
					for (auto neigbour_range_decay : neigbour_range_decays)
					{
						for (auto random_seed : random_seeds)
						{
							for (auto iterations : iterations_all)
							{
								for (auto distribution_type : distribution_types)
								{
									for (auto metric_type : metric_types)
									{
										for (auto graph_type : graph_types)
										{

											pool.execute([i, &sem, &results, &speeds, graph_type, metric_type, distribution_type, 
												iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, grid_size, random_seed]() {

												double score;

												try {

													if (graph_type == 0)
													{
														// Grid4
														metric::Grid4 graph(grid_size);

														if (metric_type == 0)
														{
															// Euclidian
															metric::Euclidian<double> distance;
															
															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 1)
														{
															// Manhatten
															metric::Manhatten<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 2)
														{
															// P_norm
															metric::P_norm<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 3)
														{
															// Euclidian_thresholded
															metric::Euclidian_thresholded<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 4)
														{
															// Cosine
															metric::Cosine<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 5)
														{
															// Chebyshev
															metric::Chebyshev<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
													}
													else if (graph_type == 1)
													{
														// Grid6
														metric::Grid6 graph(grid_size);

														if (metric_type == 0)
														{
															// Euclidian
															metric::Euclidian<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 1)
														{
															// Manhatten
															metric::Manhatten<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 2)
														{
															// P_norm
															metric::P_norm<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 3)
														{
															// Euclidian_thresholded
															metric::Euclidian_thresholded<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 4)
														{
															// Cosine
															metric::Cosine<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 5)
														{
															// Chebyshev
															metric::Chebyshev<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
													}
													else if (graph_type == 2)
													{
														// Grid8
														metric::Grid8 graph(grid_size);

														if (metric_type == 0)
														{
															// Euclidian
															metric::Euclidian<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 1)
														{
															// Manhatten
															metric::Manhatten<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 2)
														{
															// P_norm
															metric::P_norm<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 3)
														{
															// Euclidian_thresholded
															metric::Euclidian_thresholded<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 4)
														{
															// Cosine
															metric::Cosine<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 5)
														{
															// Chebyshev
															metric::Chebyshev<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
													}
													else if (graph_type == 3)
													{
														// Paley
														metric::Paley graph(grid_size);

														if (metric_type == 0)
														{
															// Euclidian
															metric::Euclidian<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 1)
														{
															// Manhatten
															metric::Manhatten<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 2)
														{
															// P_norm
															metric::P_norm<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 3)
														{
															// Euclidian_thresholded
															metric::Euclidian_thresholded<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 4)
														{
															// Cosine
															metric::Cosine<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 5)
														{
															// Chebyshev
															metric::Chebyshev<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
													}
													else if (graph_type == 4)
													{
														// LPS
														metric::LPS graph(grid_size);

														if (metric_type == 0)
														{
															// Euclidian
															metric::Euclidian<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 1)
														{
															// Manhatten
															metric::Manhatten<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 2)
														{
															// P_norm
															metric::P_norm<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 3)
														{
															// Euclidian_thresholded
															metric::Euclidian_thresholded<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 4)
														{
															// Cosine
															metric::Cosine<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 5)
														{
															// Chebyshev
															metric::Chebyshev<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
													}
													else if (graph_type == 5)
													{
														// Margulis
														metric::Margulis graph(grid_size);

														if (metric_type == 0)
														{
															// Euclidian
															metric::Euclidian<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 1)
														{
															// Manhatten
															metric::Manhatten<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 2)
														{
															// P_norm
															metric::P_norm<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 3)
														{
															// Euclidian_thresholded
															metric::Euclidian_thresholded<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 4)
														{
															// Cosine
															metric::Cosine<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
														else if (metric_type == 5)
														{
															// Chebyshev
															metric::Chebyshev<double> distance;

															if (distribution_type == 0)
															{
																// uniform_real_distribution
																std::uniform_real_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 1)
															{
																// normal_distribution
																std::normal_distribution<double> distribution(-1, 1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
															else if (distribution_type == 2)
															{
																// exponential_distribution
																std::exponential_distribution<double> distribution(1);

																score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
															}
														}
													}


												}
												catch (const std::runtime_error& e) {
													std::cout << "configuration #" << i << ": runtime error: " << e.what() << std::endl;
												}
												catch (const std::exception& e) {
													std::cout << "configuration #" << i << ": exception: " << e.what() << std::endl;
												}
												catch (...) {
													std::cout << "configuration #" << i << ": unknown error" << std::endl;
												}

												results.at(i) = score;

												sem.notify();
											});

											i++;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	for (auto grid_size : grid_sizes)
	{
		for (auto s_learn_rate : s_learn_rates)
		{
			for (auto f_learn_rate : f_learn_rates)
			{
				for (auto initial_neighbour_size : initial_neighbour_sizes)
				{
					for (auto neigbour_range_decay : neigbour_range_decays)
					{
						for (auto random_seed : random_seeds)
						{
							for (auto iterations : iterations_all)
							{
								for (auto distribution_type : distribution_types)
								{
									for (auto metric_type : metric_types)
									{
										for (auto graph_type : graph_types)
										{
											sem.wait();
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	pool.close();
	
	
	double minimal_score = INFINITY;
	std::string best_graph = "";
	std::string best_metric = "";
	std::string best_distribution = "";
				
	size_t best_grid_size;

	double best_s_learn_rate;
	double best_f_learn_rate;
				
	double best_initial_neighbour_size;

	double best_neigbour_range_decay;

	long long best_random_seed;
	
	unsigned int best_iterations;

	i = 0;
	for (auto grid_size : grid_sizes)
	{
		for (auto s_learn_rate : s_learn_rates)
		{
			for (auto f_learn_rate : f_learn_rates)
			{
				for (auto initial_neighbour_size : initial_neighbour_sizes)
				{
					for (auto neigbour_range_decay : neigbour_range_decays)
					{
						for (auto random_seed : random_seeds)
						{
							for (auto iterations : iterations_all)
							{
								for (auto distribution_type : distribution_types)
								{
									for (auto metric_type : metric_types)
									{
										for (auto graph_type : graph_types)
										{
											if (results[i] < minimal_score)
											{
												minimal_score = results[i];
												best_graph = graph_type_names[graph_type];
												best_metric = metric_type_names[metric_type];
												best_distribution = distribution_type_names[distribution_type];
												
												best_grid_size = grid_size;
												best_s_learn_rate = s_learn_rate;
												best_f_learn_rate = f_learn_rate;
												best_initial_neighbour_size = initial_neighbour_size;
												best_neigbour_range_decay = neigbour_range_decay;
												best_random_seed = random_seed;
												best_iterations = iterations;
											}
											i++;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "The best configuration: " << std::endl;
	std::cout << "  Score: " << minimal_score << std::endl;
	std::cout << "  Graph: " << best_graph << std::endl;
	std::cout << "  Distance: " << best_metric << std::endl;
	std::cout << "  Distribution: " << best_distribution << std::endl;
	std::cout << "  Grid size: " << best_grid_size << std::endl;
	std::cout << "  Iterations: " << best_iterations << std::endl;
	std::cout << "  Start learn rate: " << best_s_learn_rate << std::endl;
	std::cout << "  Final learn rate: " << best_f_learn_rate << std::endl;
	std::cout << "  Initial neighbour size: " << best_initial_neighbour_size << std::endl;
	std::cout << "  Neigbour range decay: " << best_neigbour_range_decay << std::endl;
	std::cout << "  Random seeds: " << best_random_seed << std::endl;
   
    return 0;
}
