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
	//-----------------------------------------------
	// The ground distance - thresholded Euclidean distance.
	// Since everything is ints, we multiply by COST_MULT_FACTOR.

	typedef int edm_Type;

	// const edm_Type COST_MULT_FACTOR = 1000;
	// const edm_Type THRESHOLD = 3 * COST_MULT_FACTOR; //1.412*COST_MULT_FACTOR;
	//                                                  // std::vector< std::vector<int> > cost_mat; // here it's defined as global for Rubner's interfaces.
	//                                                  // If you do not use Rubner's interface it's a better idea
	//                                                  // not to use globals.
	//                                                  // unsigned int im1_R = i0.size();
	//                                                  // unsigned int im1_C = i0.size();

	size_t im1_R = img1.size() / 6;
	size_t im1_C = img1[0].size() / 6;
	size_t n = im1_R * im1_C;

	// serialize_mat2vec
	std::vector<edm_Type> i1;
	std::vector<edm_Type> i2;
	std::vector<edm_Type> i3;
	std::vector<edm_Type> i4;

	for (size_t i = 0; i < im1_R; ++i)
	{
		for (size_t j = 0; j < im1_C; ++j)
		{
			i1.push_back(img1[i][j]);
			i2.push_back(img2[i][j]);
			i3.push_back(img3[i][j]);
			i4.push_back(img4[i][j]);
		}
	}

	auto cost_mat = metric::distance::EMD_details::ground_distance_matrix_of_2dgrid<edm_Type>(im1_C, im1_R);
	auto maxCost = metric::distance::EMD_details::max_in_distance_matrix(cost_mat);

	std::cout << "bis hier" << std::endl;

	//matrix_print(cost_mat);

	metric::distance::EMD<edm_Type> distance2(cost_mat, maxCost);

	//auto result1 = distance(v0, v2);

	auto t1 = std::chrono::steady_clock::now();

	// geht davon aus, dass i1 und i2 serialisierte Vectoren der Bildmatrizen sind und cost_mat eine passende Distanzematrix enthält, die jedoch die orgiginalen Pixelpositionen berücksichtigt.
	auto result1 = distance2(i1, i2);
	auto t2 = std::chrono::steady_clock::now();
	std::cout << "-------r" << std::endl;
	auto result2 = distance2(i2, i1);
	auto t3 = std::chrono::steady_clock::now();

	std::cout << "result1: " << result1 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;
	std::cout << "result2: " << result2 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count()) / 1000000 << "s)" << std::endl;


	/******************** examples for Structural Similartiy (for images) **************************/
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
