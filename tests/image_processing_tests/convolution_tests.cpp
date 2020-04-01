#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include <iostream>
#include "modules/utils/image_processing/convolution.hpp"


using namespace metric;


BOOST_AUTO_TEST_CASE(base)
{
	using Conv = Convolution2d<double, 3>;
	auto conv = Conv(100, 100, 4, 4);


	Conv::Channel channel;
	Conv::Image image = {channel, channel, channel};

	Conv::FilterKernel kernel = {{1, 2}, {3, 4}};

	conv(image, kernel);
}
