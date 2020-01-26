#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <limits>

#include <iostream>
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
	                               {-25, 26, 27}};
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
		file.close();
		BOOST_CHECK_EQUAL(Datasets::readDenseMatrixFromFile<double>("matrix.dsv"), m);
	}
	//BOOST_CHECK_EQUAL(Datasets::readDenseMatrixFromFile<double>("1,2,3"), m;
	//BOOST_CHECK_EQUAL(Datasets::readDenseMatrixFromFile<double>("1.2.3"), m);
	//BOOST_CHECK_EQUAL(Datasets::readDenseMatrixFromFile<double>("1 ; 2 ; 3"), m);


}
