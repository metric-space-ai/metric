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
	auto hog = HOG<double>();

	auto image = metric::Datasets::readDenseMatrixFromFile("image.pgm");
}
