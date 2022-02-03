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

#include <nlohmann/json.hpp>
#include "metric/mapping.hpp"
#include "metric/mapping/Redif.hpp"


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

///

int main(int argc, char *argv[])
{
	std::cout << "Reverse Diffusion example have started" << std::endl;
	std::cout << std::endl;

	using Record = std::vector<double>;
	using Metric = metric::Euclidean<double>;

	std::vector<Record> dataset = readCsvData("./assets/testdataset/compound.csv", ',');
	std::vector<Record> test_dataset = dataset;
	
	for (int i = 0; i < 4; ++i)
	{
		test_dataset.push_back(dataset[i]);
	}

	metric::Redif redif(dataset, 4, 10, Metric());
	
	auto encoded_data = redif.encode(test_dataset);
	auto decoded_data = redif.decode(encoded_data);	

	auto is_equal = std::equal(
		test_dataset.begin(),
		test_dataset.end(),
		decoded_data.begin(),
		[](Record l_record, Record r_record) {
			return std::equal(
						l_record.begin(),
						l_record.end(),
						r_record.begin(),
						[](double l, double r) { return (round(l * 10000) == round(r * 10000)); }
			);
		}
	);
		
	std::cout << "is encoded and decoded back dataset is equal with original: " << (is_equal ? "true" : "false") << std::endl;

    return 0;
}
