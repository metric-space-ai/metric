#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include <iostream>
#include "modules/utils/image_processing/convolution.hpp"


using namespace metric;


BOOST_AUTO_TEST_CASE(base)
{
	using T = double;
	std::normal_distribution<T> normalDistribution(0.5, 0.25);
	std::mt19937 randomEngine{std::random_device()()};

	using Conv = Convolution2d<T, 3>;
	size_t rows = 100;
	size_t columns = 100;
	auto conv = Conv(100, 100, 2, 2);


	Conv::Image image;
	for (auto c = 0; c < 3; ++c) {
		Conv::Channel channel = blaze::generate(rows, columns, [&normalDistribution, &randomEngine](size_t i, size_t j) {
			return normalDistribution(randomEngine);
		});
		image[c] = channel;
	}

	Conv::FilterKernel kernel = {{1, 2}, {3, 4}};

	conv(image, kernel);
}
