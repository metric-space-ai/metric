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
#include "metric/utils/auto_detect_metric.hpp"


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

void printDataInfo(const json& data)
{
	for (const auto& [key, value]: data.items()) {
		std::cout << key << " " << value.size() << std::endl;
	}
}

template <typename SOM, typename Record>
int checkSOM(SOM &som_model, std::vector<Record> dataset)
{
    if (!som_model.isValid()) {
    	std::cout << "SOM is not valid" << std::endl;
    	return EXIT_FAILURE;
    }
	
	std::cout << std::endl;
	/* Estimate with img1 */
    std::cout << "Estimate started..." << std::endl;
    auto t1 = std::chrono::steady_clock::now();
    som_model.estimate(dataset, 50);
    auto t2 = std::chrono::steady_clock::now();

    std::cout << "Estimate ended (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;

	auto std_deviation = som_model.std_deviation(dataset);
	std::cout << "std deviation: " << std_deviation << std::endl;
	std::cout << std::endl;


    auto dimR = som_model.encode(dataset[2]);
	vector_print(dimR, 3, 2);
	std::cout << std::endl;

	auto bmu = som_model.BMU(dataset[2]);
	std::cout << "Best matching unit: " << bmu << std::endl;

	std::cout << std::endl;

	//

	/* Train with img1 */
    std::cout << "Full train started..." << std::endl;
    t1 = std::chrono::steady_clock::now();
    som_model.train(dataset);
    t2 = std::chrono::steady_clock::now();

    std::cout << "Full train ended (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;

	std_deviation = som_model.std_deviation(dataset);
	std::cout << "std deviation: " << std_deviation << std::endl;
	std::cout << std::endl;


    dimR = som_model.encode(dataset[4]);
	vector_print(dimR, 3, 2);
	std::cout << std::endl;

	bmu = som_model.BMU(dataset[4]);
	std::cout << "Best matching unit: " << bmu << std::endl;

	std::cout << std::endl;

	//	

	std::vector<Record> test_sample = {
		{0, 3, 5, 0},
		{3, 6, 2, 1}
	};

	return 0;
}

int main()
{
	std::cout << "SOM example have started" << std::endl;
	std::cout << '\n';
	
	using Record = std::vector<double>;
    using Graph = metric::Grid6; 


	std::vector<Record> dataset = {
		{0, 3, 5, 0},
		{1, 4, 5, 0},
		{2, 5, 2, 1},
		{3, 6, 2, 1},
		{4, 7, 5, 1},
		{5, 8, 5, 1},
		{6, 9, 2, 4},
		{7, 1, 2, 4},
		{8, 9, 5, 8},
		{9, 9, 9, 8},
	};
	
	//std::vector<Record> dataset = {
	//	{0, 0, 0},
	//	{0, 0, 0},
	//	{0, 0, 0},
	//	{2, 2, 2},
	//	{2, 2, 2},
	//	{2, 2, 2},
	//	{0, 0, 4},
	//	{0, 0, 4},
	//	{0, 0, 4},
	//	{8, 0, 0},
	//	{8, 0, 0},
	//	{8, 0, 0},
	//};

	
	//

	int graph_w = 3;
	int graph_h = 2;
	Graph graph(graph_w, graph_h);
	metric::MetricAutoDetector adm;

	adm.set_verbose(true);

	auto best_metric = adm.detect<Record, Graph>(graph, graph_w, graph_h, dataset, false);
	
	if (best_metric == "Euclidean")
	{
		// Euclidean
		using Metric = metric::Euclidean<double>;
		metric::SOM<Record, Graph, Metric> som_model(graph, Metric(), 0.8, 0.2, 20);
		checkSOM(som_model, dataset);
	}
	else if (best_metric == "Manhatten")
	{
		// Manhatten
		using Metric = metric::Manhatten<double>;
		metric::SOM<Record, Graph, Metric> som_model(graph, Metric(), 0.8, 0.2, 20);
		checkSOM(som_model, dataset);
	}
	else if (best_metric == "P_norm")
	{
		// P_norm
		using Metric = metric::P_norm<double>;
		metric::SOM<Record, Graph, Metric> som_model(graph, Metric(), 0.8, 0.2, 20);
		checkSOM(som_model, dataset);
	}
	else if (best_metric == "Euclidean_thresholded")
	{
		// Euclidean_thresholded
		using Metric = metric::Euclidean_thresholded<double>;
		metric::SOM<Record, Graph, Metric> som_model(graph, Metric(), 0.8, 0.2, 20);
		checkSOM(som_model, dataset);
	}
	else if (best_metric == "Cosine")
	{
		// Cosine
		using Metric = metric::Cosine<double>;
		metric::SOM<Record, Graph, Metric> som_model(graph, Metric(), 0.8, 0.2, 20);
		checkSOM(som_model, dataset);
	}
	else if (best_metric == "Chebyshev")
	{
		// Chebyshev
		using Metric = metric::Chebyshev<double>;
		metric::SOM<Record, Graph, Metric> som_model(graph, Metric(), 0.8, 0.2, 20);
		checkSOM(som_model, dataset);
	}
	else if (best_metric == "Earth Mover Distance")
	{
		// Earth Mover Distance
		using Metric = metric::EMD<double>;
		metric::SOM<Record, Graph, Metric> som_model(graph, Metric(), 0.8, 0.2, 20);
		checkSOM(som_model, dataset);
	}
	else if (best_metric == "SSIM")
	{
		// SSIM
		using Metric = metric::SSIM<double, Record>;
		metric::SOM<Record, Graph, Metric> som_model(graph, Metric(), 0.8, 0.2, 20);
		checkSOM(som_model, dataset);
	}
	else if (best_metric == "TWED")
	{
		// TWED
		using Metric = metric::TWED<double>;
		metric::SOM<Record, Graph, Metric> som_model(graph, Metric(0, 1), 0.8, 0.2, 20);
		checkSOM(som_model, dataset);
	}
	
	//
   
    return 0;
}
