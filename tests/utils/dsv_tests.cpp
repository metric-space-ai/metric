#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <limits>

#include <iostream>
#include <chrono>

#include "modules/utils/datasets.hpp"


using namespace std;
using namespace metric;


using Matrix = blaze::DynamicMatrix<double, blaze::columnMajor>;
using Vector = blaze::DynamicVector<double>;


BOOST_AUTO_TEST_CASE(base)
{
	blaze::DynamicMatrix<double> m{{1, 2, 3},
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
		BOOST_CHECK_EQUAL(Datasets::readDenseMatrixFromFile<double>("matrix.dsv"), m);
	}

	/*auto t1 = std::chrono::high_resolution_clock::now();
	auto dm = Datasets::readDenseMatrixFromFile<double>("/tmp/d.dsv");
	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << dm << std::endl;
	std::cout << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << std::endl;
	*/
}
