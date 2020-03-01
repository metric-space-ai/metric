#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <limits>

#include <iostream>
#include <modules/utils/datasets.hpp>
#include "modules/transform/hog.hpp"


using namespace metric;

using Matrix = blaze::DynamicMatrix<double>;
using Vector = blaze::DynamicVector<double>;


BOOST_AUTO_TEST_CASE(base)
{
	auto hog = HOG<double>(9, 8, 4);

	//auto image = metric::Datasets::readDenseMatrixFromFile("image.pgm");

	blaze::DynamicMatrix<double> image = blaze::generate( 320, 320, []( size_t i, size_t j ){ return 2.1F + 1.1F*(i*3UL+j); } );
	std::cout << image << std::endl;

	std:: cout << hog.compute(image) << std::endl;
}
