/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>

#include <iostream>

#include <chrono>

#include "../../modules/mapping.hpp"




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
	if (vec.size() > 0)
	{	
		for (int i = 0; i < vec.size() - 1; i++)
		{
			std::cout << vec[i] << ", ";
		}	
		std::cout << vec[vec.size() - 1];
	}
    std::cout << " ]" << std::endl;
}


///

int main(int argc, char *argv[])
{
	std::cout << "KOC example have started" << std::endl;
	std::cout << std::endl;

	using Record = std::vector<double>;
				
	size_t best_w_grid_size = 3;
	size_t best_h_grid_size = 2;

	// if overrided from arguments
	
	if (argc > 3)
	{
		best_w_grid_size = std::stod(argv[2]);
		best_h_grid_size = std::stod(argv[3]);
	}
	
	
	std::vector<Record> dataset;
	std::vector<Record> test_set;
	std::vector<std::string> dates;

	//

	dataset = {

		{0, 0.1},
		{0.2, 0},

		{0, 1.2},
		{0.1, 1},

		{0.1, 2},
		{0.2, 2},

		{1, 0},
		{1.2, 0.1},

		{1.3, 1.1},
		{0.9, 1},

		{1.1, 2},
		{0.9, 1.9},
	};

	test_set = {
		{0.5, 0.5},
		{5, 5},
		{0, 0.1},
		{0, 1},
	};

	//
	
	///////////////////////////////////////////////////
	
	int num_clusters = 3;
	double sigma = 1.5;
		
	metric::KOC_factory<Record, metric::Grid4> simple_koc_factory(best_w_grid_size, best_h_grid_size, 0.5, 0.0, 300);    
	auto simple_koc = simple_koc_factory(dataset, num_clusters); 

	std::cout << "train dataset:" << std::endl;

	auto anomalies = simple_koc.check_if_anomaly(dataset, sigma);	
	std::cout << std::endl;
	std::cout << "anomalies:" << std::endl;
	vector_print(anomalies);

	// there are should be no anomalies in the train dataset
	for (int i = 0; i < anomalies.size(); i++)
	{
		assert(anomalies[i] == 0);
	}
	
	auto assignments = simple_koc.result(dataset, sigma);	
	std::cout << std::endl;
	std::cout << "assignments:" << std::endl;
	vector_print(assignments);

	// there are should be no zero-named clusters in the train dataset
	for (int i = 0; i < assignments.size(); i++)
	{
		assert(assignments[i] != 0);
	}
	
	//
	
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "test dataset:" << std::endl;

	anomalies = simple_koc.check_if_anomaly(test_set, sigma);	
	std::cout << std::endl;
	std::cout << "anomalies:" << std::endl;
	vector_print(anomalies);
	// first two records should be anomalies, others two should not
	assert(anomalies[0] == 1);
	assert(anomalies[1] == 1);
	assert(anomalies[2] == 0);
	assert(anomalies[3] == 0);
	
	assignments = simple_koc.result(test_set, sigma);	
	std::cout << std::endl;
	std::cout << "assignments:" << std::endl;
	vector_print(assignments);
	// first two records should zero-named anomalies
	assert(assignments[0] == 0);
	assert(assignments[1] == 0);

	//
	
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "top outliers:" << std::endl;

	auto [idxs, sorted_distances, sorted_assignments] = simple_koc.top_outlier(test_set, sigma, 10);
	
	std::cout << std::endl;
	std::cout << "sorted indexes:" << std::endl;
	vector_print(idxs);
	// indexes should be as following
	assert(idxs[0] == 1);
	assert(idxs[1] == 0);
	assert(idxs[2] == 2);
	assert(idxs[3] == 3);


	///////////////////////////////////////////////////////////////
	
	// empty test dataset
	
	std::vector<Record> empty_set;
			
	metric::KOC_factory<Record, metric::Grid4> koc_factory_1(best_w_grid_size, best_h_grid_size, 0.5, 0.0, 300);    
	auto koc_1 = koc_factory_1(dataset, num_clusters); 
	
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "empty dataset:" << std::endl;

	anomalies = koc_1.check_if_anomaly(empty_set, sigma);
	std::cout << std::endl;
	std::cout << "anomalies: " << std::endl;
	vector_print(anomalies);
	// all should be ok, but zero sized result
	assert(anomalies.size() == 0);
	
	assignments = koc_1.result(empty_set, sigma);
	std::cout << std::endl;
	std::cout << "assignments:" << std::endl;
	vector_print(assignments);
	// all should be ok, but zero sized result
	assert(assignments.size() == 0);

	//
	
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "top outliers:" << std::endl;

	std::tie(idxs, sorted_distances, sorted_assignments) = koc_1.top_outlier(empty_set, sigma, 10);
	
	std::cout << std::endl;
	std::cout << "sorted indexes:" << std::endl;
	vector_print(idxs);
	// all should be ok, but zero sized result
	assert(idxs.size() == 0);
	assert(sorted_distances.size() == 0);
	assert(sorted_assignments.size() == 0);

	

    return 0;
}
