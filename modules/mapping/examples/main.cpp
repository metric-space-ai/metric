#include <vector>

#include <iostream>
#include <fstream>

#include <chrono>

#include "assets/json.hpp"
#include "../metric_mapping.cpp"


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

int main()
{
    unsigned int iterations = 1000;
    // double start_learn_rate = 0.9;
    // double final_learn_rate = 0.4;
    // double neighborhoodSize = 1.2;

    using Vector = std::vector<double>;
    using Metric = metric::mapping::SOM_details::default_euclidian<Vector>;
    using Graph = metric::mapping::SOM_details::Grid6;

    metric::mapping::SOM<Vector, Metric, Graph> DR(6, 5);

    if (!DR.isValid()) {
    	std::cout << "SOM is not valid" << std::endl;
    	return EXIT_FAILURE;
    }

	
	/* Load data */

	std::ifstream dataFile("..\\assets\\data.json");
	
	json data;

	dataFile >> data;

	printDataInfo(data);

	const auto img1 = data["img1"].get<std::vector<std::vector<double>>>();
	const auto img2 = data["img2"].get<std::vector<std::vector<double>>>();


	/* Train with img1 */
    const auto t1 = std::chrono::steady_clock::now();
    DR.train(img1, iterations);
    const auto t2 = std::chrono::steady_clock::now();

    std::cout << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;


    auto dimR = DR.reduce(img1[0]);

    //vector_print(dimR);
	vector_print(dimR, 6, 5);


	/* Train with img2 */
    DR.train(img2, iterations);

    dimR = DR.reduce(img1[0]);
    //vector_print(dimR);
	vector_print(dimR, 6, 5);

    std::cout << std::endl;

   
    return 0;
}
