/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#include <vector>
#include <iostream>
#include <fstream>

#if defined(_WIN64)
#include <filesystem>
#endif

#include <chrono>

#include "assets/json.hpp"
#include "../../modules/mapping.hpp"
#include "../../modules/mapping/Redif.hpp"


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


std::vector<std::vector<double>> readCsvData(std::string filename, char delimeter)
{
	std::fstream fin;

	fin.open(filename, std::ios::in);
	
	std::vector<double> row;
	std::string line, word, w;

	std::vector<std::vector<double>> rows;
	std::vector<int> labels;

	// omit header
	//getline(fin, line);

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

///

int main(int argc, char *argv[])
{
	std::cout << "Reverse Diffusion example have started" << std::endl;
	std::cout << std::endl;

	using Record = std::vector<double>;
    using Metric = metric::Euclidean<double>;

	size_t best_w_grid_size = 20;
	size_t best_h_grid_size = 20;
	
	std::vector<Record> dataset = readCsvData("./assets/testdataset/compound.csv", ',');
	std::vector<Record> test_dataset;
	
	for (int i = 0; i < 4; ++i)
	{
		test_dataset.push_back(dataset[i]);
	}

	metric::Redif redif(dataset, 7, 15, Metric());
	
	auto [encoded_data, indecies] = redif.encode(test_dataset);


    return 0;
}
