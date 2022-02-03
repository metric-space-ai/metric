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

#include <nlohmann/json.hpp>
#include "metric/mapping.hpp"


using json = nlohmann::json;


template <typename T>
void matrix_print(const std::vector<std::vector<T>> &mat)
{

    std::cout << "[ " << std::endl;
    for (int i = 0; i < mat.size(); i++)
    {
		std::cout << "  [ ";
        for (int j = 0; j < mat[i].size() - 1; j++)
        {
            std::cout << mat[i][j] << ", ";
        }
        std::cout << mat[i][mat[i].size() - 1] << " ]" << std::endl;
        
    }
    std::cout << "]" << std::endl;
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

int main()
{
	std::cout << "SOM example have started" << std::endl;
	std::cout << '\n';
	
    using Vector = std::vector<double>;
    using Metric = metric::Euclidean<Vector::value_type>;
    using Graph = metric::Grid6; 

    metric::SOM<Vector, Graph, Metric> som_model(Graph(6, 5), Metric(), 0.8, 0.2, 20);

    if (!som_model.isValid()) {
    	std::cout << "SOM is not valid" << std::endl;
    	return EXIT_FAILURE;
    }

	
	/* Load data */

	std::cout << "load data" << std::endl;
	std::ifstream dataFile("assets/data.json");
	
	json data;

	try
	{
		dataFile >> data;
	}
	catch (const std::exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
	}
	catch (...) {
		std::cout << "Error: unknown" << std::endl;
	}

	std::cout << "print json" << std::endl;
	printDataInfo(data);
	std::cout << std::endl;

	const auto img1 = data["img1"].get<std::vector<std::vector<double>>>();
	const auto img2 = data["img2"].get<std::vector<std::vector<double>>>();
	
	//

	/* Estimate with img1 */
    std::cout << "Estimate started..." << std::endl;
    auto t1 = std::chrono::steady_clock::now();
    som_model.estimate(img1, 50);
    auto t2 = std::chrono::steady_clock::now();

    std::cout << "Estimate ended (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;

	auto std_deviation = som_model.std_deviation(img1);
	std::cout << "std deviation: " << std_deviation << std::endl;
	std::cout << std::endl;


    auto dimR = som_model.encode(img1[0]);
	vector_print(dimR, 6, 5);
	std::cout << std::endl;

	auto bmu = som_model.BMU(img1[0]);
	std::cout << "Best matching unit: " << bmu << std::endl;

	std::cout << std::endl;

	//

	/* Train with img1 */
    std::cout << "Full train started..." << std::endl;
    t1 = std::chrono::steady_clock::now();
    som_model.train(img1);
    t2 = std::chrono::steady_clock::now();

    std::cout << "Full train ended (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;

	std_deviation = som_model.std_deviation(img1);
	std::cout << "std deviation: " << std_deviation << std::endl;
	std::cout << std::endl;


    dimR = som_model.encode(img1[0]);
	vector_print(dimR, 6, 5);
	std::cout << std::endl;

	bmu = som_model.BMU(img1[0]);
	std::cout << "Best matching unit: " << bmu << std::endl;

	std::cout << std::endl;

	//

	/* Train with img2 */
    som_model.train(img2);

    dimR = som_model.encode(img1[0]);
	vector_print(dimR, 6, 5);
	std::cout << std::endl;

	bmu = som_model.BMU(img1[0]);
	std::cout << "Best matching unit: " << bmu << std::endl;

	std_deviation = som_model.std_deviation(img2);
	std::cout << "std deviation: " << std_deviation << std::endl;

    std::cout << std::endl;

   
    return 0;
}
