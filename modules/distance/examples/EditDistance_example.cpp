#include <vector>
#include <iostream>
#include <chrono>
#include "assets/test_data.cpp"
#include "../metric_distance.cpp"
//#include "../details/Edit.hpp"

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

int main()
{

	std::string str1 = "1011001100110011001111110011001100110011011110011001100110011001110001100110011001101";
	std::string str2 = "1000011001100110011011100110011001100110111001100110011001100111000110011001100110011";

	// std::vector<int> vec2 = {1,2,3,4,5,6,7,8,9,10};
	// std::vector<int> vec1 = {};

	metric::distance::Edit<std::string> distance3;

	auto t3 = std::chrono::steady_clock::now();
	auto result3 = distance3(str1, str2);
	auto t4 = std::chrono::steady_clock::now();

	std::cout << "result3: " << result3 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count()) / 1000000 << "s)" << std::endl;


	/******************** examples for Edit distance (for strings) **************************/
	// example for number
	// example for string
	// example for date
	// example for array
	// example for curve
	// example for sentence
	// example for soundwave
	// example for picture

	return 0;
}
