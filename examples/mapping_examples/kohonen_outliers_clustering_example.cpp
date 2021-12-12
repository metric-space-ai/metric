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

#include "modules/utils/ThreadPool.hpp"
#include "modules/utils/Semaphore.h"

#include <nlohmann/json.hpp>
#include "modules/mapping.hpp"


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

std::vector<std::vector<double>> readCsvData(std::string filename, char delimeter)
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
		std::stringstream s(line);

		row.clear();
		while (getline(s, word, delimeter))
		{
			row.push_back(std::stod(word));
		}

		rows.push_back(row);
	}

	return rows;
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

	std::vector<std::vector<Record>> datasets;
	std::vector<std::vector<Record>> test_sets;
	std::vector<std::string> dataset_names;
	
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
	datasets.push_back(dataset);

	test_set = {
		{0, 0},
		{0, 1},
		{0.5, 0.5},
		{0.0, 0.3},
		{5, 5},
	};
	test_sets.push_back(test_set);

	dataset_names.push_back("syntetic dataset");

	//
	
	dataset = readCsvData("assets/testdataset/compound.csv", ',');
	datasets.push_back(dataset);

	test_set = { {4, 0} };
	test_sets.push_back(test_set);

	dataset_names.push_back("compound dataset");

	//

	dataset = readCsvData("assets/testdataset/fisheriris.csv", ',');
	datasets.push_back(dataset);

	test_set = {
		{6.5, 3.2, 5.1, 2.2},
		{6.1, 3.3, 5.3, 2.3},
		{5.9, 3.1, 5.2, 1.8},
		{8.3, 5.0, 5.0, 3.5}
	};
	test_sets.push_back(test_set);

	dataset_names.push_back("fisheriris dataset");

	//

	dataset = readCsvData("assets/testdataset/multidim.csv", ',');
	datasets.push_back(dataset);

	test_set = {
		{1.86,-0.5,0.01,0.36,-0.04,-0.35,0.11,0.09,0.57,-0.09,-0.03,0.05,-0.21,-0.21,0.04,-0.14,0.14,-0.11,0.18,-0.06,-0.04,0.08,-0.03,-0.13,0.11,0.02,0.04,-0.04,-0.14,-0.1},
		{0.87,-0.97,0.24,0.18,0.23,0.35,0.21,-0.06,0.01,0.06,-0.1,0.02,-0.13,0.18,-0.43,0.06,-0.24,0.12,0.04,-0.2,-0.12,0.23,0.06,0.2,-0.09,0.01,0.28,0.01,0.11,-0.04},
		{2,2,0.4,0.01,-0.1,0.1,0.2,0.1,-0.1,0.4,-0.1,-0.01,0.3,0.2,0.3,0.01,0.2,0.1,-0.1,-0.1,-0.4,0.1,0.2,0.3,0.01,0.3,0.1,0.4,0.1,0.1}
	};
	test_sets.push_back(test_set);

	dataset_names.push_back("multidim dataset");

	//

	std::tie(dates, dataset) = readCsvData2("assets/testdataset/nyc_taxi.csv", ',');
	datasets.push_back(dataset);

	test_set = { dataset[2], dataset[12], dataset[22], dataset[53], dataset[84], dataset[122], dataset[123], dataset[149], dataset[162], dataset[163],
		dataset[183], dataset[184], dataset[192], dataset[199], dataset[201]  };
	test_sets.push_back(test_set);

	dataset_names.push_back("NYC taxi dataset");

	//

	dataset = readCsvData("assets/testdataset/3d_swissroll.csv", ',');
	datasets.push_back(dataset);

	test_set = { dataset[2], dataset[12], dataset[22]  };
	test_sets.push_back(test_set);

	dataset_names.push_back("swissroll dataset");

	///

	// random seed for repeateable results
	long long random_seed = 777;
	double sigma = 1.5;

	metric::KOC_factory<Record, metric::Grid4> simple_koc_factory(best_w_grid_size, best_h_grid_size, sigma, 0.5, 0.0, 300, -1, 1, 2, 0.5, random_seed);

	for (int i = 0; i < datasets.size(); i++)
	{
		std::cout << "--------------" << std::endl;
		std::cout << dataset_names[i] << std::endl;
		std::cout << "--------------" << std::endl;
		std::cout << std::endl;

		int num_clusters = 5;

		dataset = datasets[i];
		test_set = test_sets[i];

		auto simple_koc = simple_koc_factory(dataset, num_clusters);


		std::cout << "train dataset:" << std::endl;

		auto anomalies = simple_koc.check_if_anomaly(dataset);
		std::cout << std::endl;
		std::cout << "anomalies:" << std::endl;
		vector_print(anomalies);

		auto assignments = simple_koc.assign_to_clusters(dataset);
		std::cout << std::endl;
		std::cout << "assignments:" << std::endl;
		vector_print(assignments);


		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << "test dataset:" << std::endl;

		anomalies = simple_koc.check_if_anomaly(test_set);
		std::cout << std::endl;
		std::cout << "anomalies:" << std::endl;
		vector_print(anomalies);

		assignments = simple_koc.assign_to_clusters(test_set);
		std::cout << std::endl;
		std::cout << "assignments:" << std::endl;
		vector_print(assignments);

		///

		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << "top outliers:" << std::endl;

		auto [idxs, sorted_distances] = simple_koc.top_outliers(test_set, 10);

		std::cout << std::endl;
		std::cout << "sorted indexes:" << std::endl;
		vector_print(idxs);

		std::cout << std::endl;
		std::cout << "sorted distances:" << std::endl;
		vector_print(sorted_distances);

		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;
	}

    return 0;
}
