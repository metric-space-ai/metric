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
#include "modules/mapping.hpp"
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
            std::cout << static_cast<unsigned int>(mat[i][j]) << ", ";
        }
        std::cout << static_cast<unsigned int>(mat[i][mat[i].size() - 1]) << " ]" << std::endl;
        
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

	// load mnist dataset

	std::string MNIST_DATA_LOCATION = "assets/mnist";

    mnist::MNIST_dataset<std::vector, std::vector<double>, uint8_t> dataset =
        mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION);

    std::cout << "Number of training images = " << dataset.training_images.size() << std::endl;
    std::cout << "Number of training labels = " << dataset.training_labels.size() << std::endl;
    std::cout << "Number of test images = " << dataset.test_images.size() << std::endl;
    std::cout << "Number of test labels = " << dataset.test_labels.size() << std::endl;

	
    std::cout << "Test label 0 = " << static_cast<unsigned int>(dataset.training_labels[0]) << std::endl;
    std::cout << "Test label 0 = " << static_cast<unsigned int>(dataset.training_labels[1]) << std::endl;
    std::cout << "Test label 0 = " << static_cast<unsigned int>(dataset.training_labels[2]) << std::endl;

	
    std::cout << "Training image 0 = " << dataset.training_images[0].size() << std::endl;
	
	for (auto i = 0; i < 5; i++)
	{
		std::cout << std::hex;
		image_print(dataset.training_images[i], 28, 28);	
		std::cout << std::dec << std::endl;
	}
	
	for (auto i = 0; i < 20; i++)
	{
		std::cout << "Test label " << i << " = " << static_cast<unsigned int>(dataset.training_labels[i]) << std::endl;
	}

	//
	
	int grid_w = 5;
	int grid_h = 4;
	
    using Vector = std::vector<double>;
    using Metric = metric::Euclidean<double>;
    //using Metric = metric::SSIM<double, std::vector<double>>;
    using Graph = metric::Grid6; 
	std::uniform_real_distribution<double> distr(0, 255);


	Metric distance;
	
	
	std::vector<std::vector<double>> train_images = dataset.training_images;
	//std::vector<std::vector<std::vector<double>>> train_images;
	//
	//for (auto i = 0; i < 5; i++)
	//{
	//	std::vector<std::vector<double>> image(28, std::vector<double>());
	//	for (auto p = 0; p < dataset.training_images[i].size(); p++)
	//	{
	//		image[(int) (p / 28)].push_back(dataset.training_images[i][p]);
	//	}
	//	train_images.push_back(image);
	//}
	
	
	std::vector<std::vector<double>> test_images = dataset.test_images;
	//std::vector<std::vector<std::vector<double>>> test_images;
	//for (auto i = 0; i < 5; i++)
	//{
	//	std::vector<std::vector<double>> image(28, std::vector<double>());
	//	for (auto p = 0; p < dataset.test_images[i].size(); p++)
	//	{
	//		image[(int) (p / 28)].push_back(dataset.test_images[i][p]);
	//	}
	//	test_images.push_back(image);
	//}
	
	//std::cout << std::hex;
	//matrix_print(train_images[0]);
	//matrix_print(train_images[1]);
	//std::cout << std::dec << std::endl;

	std::cout << "result: " << distance(train_images[0], train_images[1]) << std::endl;
	std::cout << "" << std::endl;


    metric::SOM<Vector, Graph, Metric> som_model(Graph(grid_w, grid_h), Metric(), 0.8, 0.2, 20, distr);

    if (!som_model.isValid()) {
    	std::cout << "SOM is not valid" << std::endl;
    	return EXIT_FAILURE;
    }


	/* Estimate with img1 */
    //std::cout << "Estimate started..." << std::endl;
    //auto t1 = std::chrono::steady_clock::now();
    //som_model.estimate(dataset.training_images, 50);
    //auto t2 = std::chrono::steady_clock::now();

    //std::cout << "Estimate ended (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;
	

	/* Train with img1 */
    std::cout << "Full train started..." << std::endl;
    auto t1 = std::chrono::steady_clock::now();
    som_model.train(train_images);
    auto t2 = std::chrono::steady_clock::now();

    std::cout << "Full train ended (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;

	auto nodes_data = som_model.get_weights();
	
	// clustering on the reduced data
	
    //metric::Matrix<std::vector<double>, metric::Cosine<double>> distance_matrix(nodes_data);	
    //auto [assignments, exemplars, counts] = metric::affprop(distance_matrix, (float)0.25);
    auto [assignments, exemplars, counts] = metric::kmeans(nodes_data, 10, 1000);


	std::cout << "assignments:" << std::endl;
	vector_print(assignments, grid_w, grid_h);
	std::cout << std::endl;

	std::cout << "counts:" << std::endl;
	vector_print(counts);
	std::cout << std::endl;

	std::vector<std::vector<double>> result(grid_w * grid_h, std::vector<double>(10, 0));

	for (auto i = 0; i < train_images.size(); i++)
	{
		auto bmu = som_model.BMU(train_images[i]);
		result[bmu][dataset.training_labels[i]]++;
	}

	matrix_print(result);

	std::vector<double> digits;
	for (auto i = 0; i < result.size(); i++)
	{
		auto r = std::max_element(result[i].begin(), result[i].end());
		digits.push_back(std::distance(result[i].begin(), r));
	}
	vector_print(digits);

	int matches = 0;
	int errors = 0;
	for (auto i = 0; i < test_images.size(); i++)
	{
		auto bmu = som_model.BMU(test_images[i]);
		if (digits[bmu] == dataset.test_labels[i])
		{
			matches++;
		}
		else
		{
			errors++;
		}
	}
	std::cout << "matches: " << matches << " errors: " << errors << " accuracy: " << (double) matches / ((double) matches + (double) errors) << std::endl;

	//

	///* Train with img1 */
 //   std::cout << "Full train started..." << std::endl;
 //   t1 = std::chrono::steady_clock::now();
 //   som_model.train(img1);
 //   t2 = std::chrono::steady_clock::now();

 //   std::cout << "Full train ended (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;

	//std_deviation = som_model.std_deviation(img1);
	//std::cout << "std deviation: " << std_deviation << std::endl;
	//std::cout << std::endl;


 //   dimR = som_model.encode(img1[0]);
	//vector_print(dimR, 6, 5);
	//std::cout << std::endl;

	//bmu = som_model.BMU(img1[0]);
	//std::cout << "Best matching unit: " << bmu << std::endl;

	//std::cout << std::endl;

	////

	///* Train with img2 */
 //   som_model.train(img2);

 //   dimR = som_model.encode(img1[0]);
	//vector_print(dimR, 6, 5);
	//std::cout << std::endl;

	//bmu = som_model.BMU(img1[0]);
	//std::cout << "Best matching unit: " << bmu << std::endl;

	//std_deviation = som_model.std_deviation(img2);
	//std::cout << "std deviation: " << std_deviation << std::endl;

 //   std::cout << std::endl;

   
    return 0;
}
