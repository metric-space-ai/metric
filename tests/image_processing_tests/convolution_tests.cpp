#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include <iostream>
#include "modules/utils/image_processing/convolution.hpp"


using namespace metric;


BOOST_AUTO_TEST_CASE(base)
{
	auto conv = Convolution2d<double, 3>(100, 100, 4, 4);
}
