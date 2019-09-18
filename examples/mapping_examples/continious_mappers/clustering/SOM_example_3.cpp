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

	// int i = 0;
	while (getline(fin, line))
	{
		// i++;
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

void saveToCsv(std::string filename, const std::vector<std::vector<std::string>> &mat, const std::vector<std::string> &features)
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
	std::cout << std::endl;
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
	std::cout << "configuration #" << i << " finished" << std::endl;
	std::cout << "  Graph: " << typeid(graph).name() << std::endl;
	std::cout << "  Distance: " << typeid(distance).name() << std::endl;
	std::cout << "  Distribution: " << typeid(distribution).name() << std::endl;
	std::cout << "Total distances: " << total_distances << 
		" mean distance: " << total_distances / data.size() << 
		" (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl << std::endl;
	mu.unlock();

	return total_distances;
}


template <typename T>
std::vector<std::vector<T>>
set2conf(std::vector<T> set_0, size_t windowSize, size_t samples, T confidencelevel)
{

  std::random_device rd;  //seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<> dis(T(0), T(1));

  // propabiliy vector
  std::vector<T> prob_0 = ooc_functions::linspace(T(1) / T(set_0.size()), T(1) - T(1) / T(set_0.size()), set_0.size());
  std::sort(set_0.begin(), set_0.end());
  //ooc_functions::dualPivotSort(set_0);

  // compute probability matrix of set_0
  std::vector<std::vector<T>> m_0(samples, std::vector<T>(set_0.size()));
  //std::vector<std::vector<T>> m_prop_0(samples, std::vector<T>(set_0.size()));

  for (size_t i = 0; i < samples; ++i)
  {
    for (size_t j = 0; j < set_0.size(); ++j)
    {
      m_0[i][j] = T(dis(gen)); // fill with random numbers
    }

    std::sort(m_0[i].begin(), m_0[i].end()); // sort the row
    //ooc_functions::dualPivotSort(m_prop_0[i]);
    m_0[i] = ooc_functions::akimaInterp1(prob_0, set_0, m_0[i]); // interpolate the random numbers
  }

  // m_prop_0.clear();
  // m_prop_0.shrink_to_fit();

  // transpose
  auto m_0t = ooc_functions::transpose(m_0);

  m_0.clear();
  m_0.shrink_to_fit();

  // compute left and right confidence boundaries of set_0
  std::vector<T> set_0_left(set_0.size());
  std::vector<T> set_0_right(set_0.size());
  for (size_t i = 0; i < set_0.size(); ++i)
  {
    set_0_left[i] = ooc_functions::quickQuantil(m_0t[i], (T(1) - confidencelevel) / T(2));
    set_0_right[i] = ooc_functions::quickQuantil(m_0t[i], confidencelevel + (T(1) - confidencelevel) / T(2));
  }

    m_0t.clear();
  m_0t.shrink_to_fit();

  // compute probability matrix of left and right and medians of set_0
  std::vector<std::vector<T>> m_prop_1(samples, std::vector<T>(windowSize));


  for (size_t i = 0; i < samples; ++i)
  {
    for (size_t j = 0; j < windowSize; ++j)
    {
      m_prop_1[i][j] = T(dis(gen)); // fill with random numbers
    }

    std::sort(m_prop_1[i].begin(), m_prop_1[i].end()); // sort the row
  }

  std::vector<std::vector<T>> quants(3, std::vector<T>(windowSize));


  // left
  std::vector<std::vector<T>> m(samples, std::vector<T>(windowSize));
  for (size_t i = 0; i < samples; ++i)
  {
      m[i] = ooc_functions::akimaInterp1(prob_0, set_0_left, m_prop_1[i]); // interpolate the random numbers
  }

  // set_0_left.clear();
  // set_0_left.shrink_to_fit();

  auto mt = ooc_functions::transpose(m);

  for (size_t i = 0; i < windowSize; ++i)
  {
    quants[0][i] = ooc_functions::quickQuantil(mt[i], (T(1.0) - confidencelevel) / T(2.0));
  }

  //right
  for (size_t i = 0; i < samples; ++i)
  {
      m[i] = ooc_functions::akimaInterp1(prob_0, set_0_right, m_prop_1[i]);
  }

  // set_0_right.clear();
  // set_0_right.shrink_to_fit();

  mt = ooc_functions::transpose(m);

  for (size_t i = 0; i < windowSize; ++i)
  {
    quants[2][i] = ooc_functions::quickQuantil(mt[i], confidencelevel + (T(1.0) - confidencelevel) / T(2.0));
  }

  //median
  for (size_t i = 0; i < samples; ++i)
  {
      m[i] = ooc_functions::akimaInterp1(prob_0, set_0, m_prop_1[i]);
  }

  mt = ooc_functions::transpose(m);
  // m.clear();
  // m.shrink_to_fit();

  // m_prop_1.clear();
  // m_prop_1.shrink_to_fit();

  for (size_t i = 0; i < windowSize; ++i)
  {
    quants[1][i] = ooc_functions::quickQuantil(mt[i], T(0.5));
  }

  return quants;
}

template <typename T>
std::vector<std::vector<std::vector<T>>>
set2multiconf(std::vector<T> set_0, std::vector<uint32_t> windowSizes, size_t samples, T confidencelevel)
{
  std::vector<std::vector<std::vector<T>>> multiquants;
  for (size_t i = 0; i < windowSizes.size(); ++i)
  {
    multiquants.push_back(set2conf(set_0, windowSizes[i], samples, confidencelevel));
  }

  return multiquants;
}

int main()
{
	std::cout << "SOM example have started" << std::endl;
	std::cout << '\n';

	/* Load data */

	readData("assets/energies_speed_190820.log");

	std::ifstream in("assets/reference_data.json.txt");
	json speeds_json;
	in >> speeds_json;
	   
	std::cout << speeds_json.size() << std::endl;

	for (auto& [key, value] : speeds_json.items()) {
		std::cout << value["id"] << "\n";
		for (auto& [key2, value2] : value["data"].items()) {
			std::cout << "  name: " << value2["name"] << "\n";
			std::cout << "  position: " << value2["position"].size() << "\n";
			std::cout << "  border: " << value2["border"].size() << "\n";
			std::cout << "  quant: " << value2["quant"].size() << "x" << value2["quant"][0].size() << "x" << value2["quant"][0][0].size() << "\n";
			
			for (auto& [key3, value3] : value2["quant"][0][0].items()) {
			std::cout << "    " << key3 << "\n";
			}
		}
	}

	
	//// parse data to C++ types
	//std::vector<float> set_0 = convertToFloatVector(env, argv[0]);
	//std::vector<uint32_t> windowSizes = convertToIntVector(env, argv[1]);
	//uint32_t samples = convertToInt(env, argv[2]);
	//float confidencelevel = convertToFloat(env, argv[3]);

	//std::vector<std::vector<std::vector<float>>> multiquants = set2multiconf(set_0, windowSizes, samples, confidencelevel);
   
    return 0;
}
