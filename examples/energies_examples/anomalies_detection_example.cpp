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
#include "../../modules/distance.hpp"


using json = nlohmann::json;

///////////////////////////////////////////////////////

std::string RAW_DATA_DIRNAME = "assets/data";

////////////////////////////////////////////////////////


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

std::vector<std::vector<double>> readEnergies(std::string dirname)
{
	#if defined(__linux__)
		std::vector<std::string> files;
		DIR *dp;
		struct dirent *dirp;
		if((dp  = opendir(dirname.c_str())) == NULL) {
			std::cout << "Error(" << errno << ") opening " << dirname << std::endl;
			return std::vector<std::vector<double>>();
		}

		while ((dirp = readdir(dp)) != NULL) {
			std::string fn = std::string(dirp->d_name);
			if (fn.size() > 4 && fn.substr(fn.size() - 4) == ".log")
			{
				files.push_back(dirname + "/" + fn);
			}
		}
		closedir(dp);
	#endif
	
	std::vector<double> row;
	std::vector<double> speeds;
	std::string line, word, w;

	std::vector<std::vector<double>> rows;
	
	#if defined(_WIN64)
		for (const auto & entry : std::filesystem::directory_iterator(dirname))
	#endif
	#if defined(__linux__)
		for (auto filename : files)
	#endif
    {
		#if defined(_WIN64)
			auto filename = entry.path();
		#endif
		std::cout << "reading data from " << filename << "... " << std::endl;

		std::fstream fin;

		fin.open(filename, std::ios::in);

		char delimeter = 9;

		int i = 0;
		while (getline(fin, line))
		{
			std::stringstream s(line);

			row.clear();
			// omit first digit
			getline(s, word, delimeter);

			while (getline(s, word, delimeter))
			{
				// std::cout << " -> " << word << std::endl;

				row.push_back(std::stold(word));
			}
			// erase last element
			double speed = row[row.size() - 1];
			speeds.push_back(speed);
			
			row.pop_back();

			if (speed >= 1)
			{
				for (auto k = 0; k < row.size(); k++)
				{
					row[k] = sqrt(row[k]);
				}
			}
			else
			{
				for (auto k = 0; k < row.size(); k++)
				{
					row[k] = 0;
				}
			}

			rows.push_back(row);
		}

		rows.pop_back();
	}

	return rows;
}


std::mutex mu;

int main(int argc, char *argv[])
{
	std::cout << "SOM example have started" << std::endl;
	std::cout << '\n';

	auto t1 = std::chrono::steady_clock::now();

	if (argc > 1)
	{
		RAW_DATA_DIRNAME = argv[1];
	}

	/* Load data */
	auto speeds = readEnergies(RAW_DATA_DIRNAME);
	std::cout << "" << std::endl;
	std::cout << "Num records: " << speeds.size() << std::endl;
	std::cout << "Num values in the record: " << speeds[0].size() << std::endl;


	unsigned concurentThreadsSupported = std::thread::hardware_concurrency();
	std::cout << "Num cores: " << concurentThreadsSupported << std::endl;
	ThreadPool pool(concurentThreadsSupported);
	Semaphore sem;
				
	size_t best_w_grid_size = 30;
	size_t best_h_grid_size = 20;

	// if overrided from arguments
	
	if (argc > 3)
	{
		best_w_grid_size = std::stod(argv[2]);
		best_h_grid_size = std::stod(argv[3]);
	}

	// create, train SOM over the raw data and reduce the data	
	// then make clustering on the reduced data
	
	int min_cluster_size = 0;
	int num_clusters = best_w_grid_size * best_h_grid_size / 2;
	
	metric::Grid4 graph(best_w_grid_size, best_h_grid_size);
	metric::Euclidian<double> distance;
	std::uniform_real_distribution<double> distribution;
	metric::SOM<std::vector<double>, metric::Grid4, metric::Euclidian<double>, std::uniform_real_distribution<double>> 
		som(graph, distance, 0.8, 0.0, 20, distribution);	

	som.train(speeds);

	auto nodes_data = som.get_weights();

	while (min_cluster_size <= 1)
	{
		// clustering on the reduced data
	
		//metric::Matrix<std::vector<double>, metric::Cosine<double>> distance_matrix(nodes_data);	
		//auto [assignments, exemplars, counts] = metric::affprop(distance_matrix, (float)0.25);
		auto [assignments, exemplars, counts] = metric::kmeans(nodes_data, num_clusters, 1000);


		std::cout << "assignments:" << std::endl;
		vector_print(assignments, best_w_grid_size, best_h_grid_size);
		std::cout << std::endl;

		std::cout << "counts:" << std::endl;
		vector_print(counts);
		std::cout << std::endl;


		// split and reshape raw data by clusters [sensor -> cluster -> energy -> values]
	
		//int num_sensors = 8;
		//int num_levels = 7;
		//std::vector<std::vector<std::vector<std::vector<double>>>> clustered_energies(num_sensors, std::vector<std::vector<std::vector<double>>>(counts.size(), std::vector<std::vector<double>>(num_levels)));

		//std::vector<int> total(assignments.size());
		//for (auto record : speeds)
		//{
		//	// find cluster id for a record
		//	auto bmu = som.BMU(record);
		//	auto cluster_index = assignments[bmu];
		//	total[bmu]++;
		//	for (int i = 0; i < num_sensors; i++)
		//	{			
		//		for (int j = 0; j < num_levels; j++)
		//		{
		//			clustered_energies[i][cluster_index][j].push_back(record[i*num_levels + j]);
		//		}
		//	}
		//}
	
		//std::cout << "cluster sizes:" << std::endl;
		//for (int k = 0; k < clustered_energies[0].size(); k++)
		//{
		//	std::cout << "cluster_index: " << k << ", size: "<< clustered_energies[0][k][0].size() << std::endl;
		//}
		//std::cout << std::endl;
		//
		//std::cout << "som nodes sizes:" << std::endl;
		//vector_print(total, w_grid_size, h_grid_size);
		//std::cout << std::endl;
		
		std::vector<int>::iterator result = std::min_element(counts.begin(), counts.end());
		min_cluster_size = counts[std::distance(counts.begin(), result)];	
		
		std::cout << "min cluster size: " << min_cluster_size << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;

		int num_ones = 0;
		for (auto i : counts)
		{
			if (i <= 1)
			{
				num_ones++;
			}
		}

		int new_num_clusters = num_clusters * min(0.5, 0.1 * num_ones);
		if (new_num_clusters == num_clusters)
		{
			new_num_clusters--;
		}

		num_clusters = new_num_clusters;

		if (num_clusters <= 1)
			break;
	}

	
	std::vector<std::vector<double>> test_samples;

	
	std::vector<double> test_sample;
	for (auto i = 0; i < 7 * 8; i++)
	{
		test_sample.push_back(((double) rand() / (RAND_MAX)));
	}
	test_samples.push_back(test_sample);

	//auto anomalies = som.check_if_anomaly(test_samples);
	//
	//std::cout << std::endl;
	//std::cout << "anomalies:" << std::endl;
	//vector_print(anomalies);
	//std::cout << std::endl;
	
	
	std::vector<double> entropies;
	std::vector<double> closest;
	double closest_sum = 0;
	for (size_t i = 0; i < speeds.size(); i++)
	{
		std::vector<std::vector<double>> dimR;
		auto dimRRR = som.encode(speeds[i]);
		for (auto d : dimRRR)
		{
			dimR.push_back({ d });
		}
		auto bmu = som.BMU(speeds[i]);
		closest.push_back(dimRRR[bmu]);
		closest_sum += dimRRR[bmu];
		
		auto e = entropy(dimR, 3, 2.0, metric::Euclidian<double>());
		entropies.push_back(e);
	}
	
    std::sort(closest.begin(), closest.end());
    std::sort(entropies.begin(), entropies.end());
	
	std::vector<double>::iterator result = std::min_element(closest.begin(), closest.end());		
	std::cout << "min closest is : " << closest[std::distance(closest.begin(), result)] << std::endl;
	result = std::max_element(closest.begin(), closest.end());		
	std::cout << "max closest is : " << closest[std::distance(closest.begin(), result)] << std::endl;
	std::cout << "mean closest is : " << closest_sum / closest.size() << std::endl;
	for (auto i = closest.size() - 10; i < closest.size(); i++)
	{
		std::cout << "last closest is : " << closest[i] << std::endl;
	}
	std::cout << std::endl;
	
	result = std::min_element(entropies.begin(), entropies.end());		
	std::cout << "min entropy is : " << entropies[std::distance(entropies.begin(), result)] << std::endl;
	result = std::max_element(entropies.begin(), entropies.end());		
	std::cout << "max entropy is : " << entropies[std::distance(entropies.begin(), result)] << std::endl;
	for (auto i = entropies.size() - 10; i < entropies.size(); i++)
	{
		std::cout << "last entropy is : " << entropies[i] << std::endl;
	}

	
	std::vector<std::vector<double>> dimR;
	auto dimRRR = som.encode(test_sample);
	for (auto d : dimRRR)
	{
		dimR.push_back({ d });
	}
	auto bmu = som.BMU(test_sample);
	std::cout << "closest : " << dimRRR[bmu] << std::endl;
		
	auto e = entropy(dimR, 3, 2.0, metric::Euclidian<double>());
	std::cout << "entropy : " << e << std::endl;

	auto t2 = std::chrono::steady_clock::now();
	std::cout << "(Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;


    return 0;
}
