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
		std::cout << "row " << i << std::endl;
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

int main()
{
	std::cout << "SOM example have started" << std::endl;
	std::cout << '\n';

	/* Load data */
	auto speeds = readData(".\\assets\\energies_speed_190820.log");
	
	for (size_t s = 0; s < 5; s++)
	{
		unsigned int iterations = 1000;
		// double start_learn_rate = 0.9;
		// double final_learn_rate = 0.4;
		// double neighborhoodSize = 1.2;

		using Vector = std::vector<double>;
		using Metric = metric::Euclidian<Vector::value_type>;
		using Graph = metric::Grid6;

		metric::SOM<Vector, Metric, Graph> DR(5 + s, 5 + s);

		if (!DR.isValid()) {
			std::cout << "SOM is not valid" << std::endl;
			return EXIT_FAILURE;
		}

		/* Train */
		std::cout << "Training... " << std::endl;
		auto t1 = std::chrono::steady_clock::now();
		DR.train(speeds, iterations);
		auto t2 = std::chrono::steady_clock::now();

		std::cout << "... done! (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;
		std::cout << '\n';

		double total_distances = 0;
		for (size_t i = 0; i < speeds.size(); i++)
		{
			auto dimR = DR.reduce(speeds[i]);
			auto bmu = DR.BMU(speeds[i]);
			total_distances += dimR[bmu];
		}
		
		t2 = std::chrono::steady_clock::now();
		std::cout << "Total distances: " << total_distances << 
			" mean distance: " << total_distances / speeds.size() << 
			" (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;
		
		std::cout << '\n';
		std::cout << '\n';
		std::cout << '\n';
	}
   
    return 0;
}
