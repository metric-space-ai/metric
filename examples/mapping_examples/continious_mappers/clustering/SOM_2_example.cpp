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
#include "../../assets/mnist/mnist_reader.hpp"


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

	for (auto index = 0; index < vec.size(); ++index) {
		std::cout << vec[index] << " ";

		if ((index + 1) % width == 0) {
			std::cout << std::endl;
		}
	}
}

template <typename T>
void image_print(const std::vector<T> &vec, const size_t width, const size_t height)
{
	if ((width * height) != vec.size()) {
		std::cout << "width * height != vector.size()" << std::endl;
		return;
	}
	
	int max_digits = 2;

	for (auto index = 0; index < vec.size(); ++index) {
		
		int pos = 10;
		int digits_num = 1;
		if (vec[index] > 15)
		{
			digits_num++;
		}
		for (auto i = 0; i < max_digits - digits_num; ++i) {
			std::cout << " ";
		}
		std::cout << static_cast<unsigned int>(vec[index]) << " ";

		if ((index + 1) % width == 0) {
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;
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

	//
	
	int grid_w = 8;
	int grid_h = 6;
	
    using Vector = std::vector<double>;
    using Metric = metric::Euclidean<double>;
    using Graph = metric::Grid6; 
	std::uniform_real_distribution<double> distr(-1, 1);

	Metric distance;	

    metric::SOM<Vector, Graph, Metric> som_model(Graph(grid_w, grid_h), Metric(), 0.8, 0.2, 20, distr);

    if (!som_model.isValid()) {
    	std::cout << "SOM is not valid" << std::endl;
    	return EXIT_FAILURE;
    }
	
	for (auto i = 0; i < 6; i ++)
	{

		/* Load data */

		std::cout << "load data" << std::endl;
		std::ifstream dataFile("assets/toy_dataset_" + std::to_string(i) + ".json");
	
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
		std::vector<std::vector<double>> train_dataset = data.get<std::vector<std::vector<double>>>();

		/* Estimate with img1 */
		//std::cout << "Estimate started..." << std::endl;
		//auto t1 = std::chrono::steady_clock::now();
		//som_model.estimate(dataset.training_images, 50);
		//auto t2 = std::chrono::steady_clock::now();

		//std::cout << "Estimate ended (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;
	

		/* Train with img1 */
		std::cout << "Full train started..." << std::endl;
		auto t1 = std::chrono::steady_clock::now();
		som_model.train(train_dataset);
		auto t2 = std::chrono::steady_clock::now();

		std::cout << "Full train ended (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;

		auto nodes_data = som_model.get_weights();
	
		// clustering on the reduced data
	
		//metric::Matrix<std::vector<double>, metric::Cosine<double>> distance_matrix(nodes_data);	
		//auto [assignments, exemplars, counts] = metric::affprop(distance_matrix, (float)0.25);
		auto [assignments, exemplars, counts] = metric::kmeans(nodes_data, 3, 1000);


		std::cout << "assignments:" << std::endl;
		vector_print(assignments, grid_w, grid_h);
		std::cout << std::endl;

		std::cout << "counts:" << std::endl;
		vector_print(counts);
		std::cout << std::endl;

	}

   
    return 0;
}
