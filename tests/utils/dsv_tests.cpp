#include <catch2/catch.hpp>

#include <limits>
#include <iostream>

#include "metric/utils/datasets.hpp"


using namespace std;
using namespace metric;


TEMPLATE_TEST_CASE("base","[utils]", float, double)
{
	using Matrix = blaze::DynamicMatrix<TestType, blaze::columnMajor>;
	using Vector = blaze::DynamicVector<TestType>;

	blaze::DynamicMatrix<TestType> m{{1, 2, 3},
	                               {4, 5, 6},
	                               {7, 8, 9},
	                               {1, 1.1, 1.2},
	                               {1.3, 1.4, -1.5},
	                               {16, -17, 18},
	                               {-0.19, 2, 0.21},
	                               {-22, 23, 24},
	                               {-25, 26, 27},
	                               {-2.8e-8, 29, 3e7}};
	{
		std::ofstream file("matrix.dsv");
		file << "1;2; 3" << std::endl;
		file << "4 , 5,6" << std::endl;
		file << "7 8  9" << std::endl;
		file << "1. , 1.1 ,1.2" << std::endl;
		file << "1,3  1,4 -1,5" << std::endl;
		file << "16 ,-17,18" << std::endl;
		file << "-,19 2 ,21" << std::endl;
		file << "-22 , 23 ,24" << std::endl;
		file << "[-25,26,27] " << std::endl;
		file << " [-2,8e-8 29 3,0e007] " << std::endl;
		file.close();

		REQUIRE(Datasets::readDenseMatrixFromFile<TestType>("matrix.dsv") == m);
	}
}
