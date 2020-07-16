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

	size_t kernelWidth = 3;
	size_t kernelHeight = 3;

	Conv::FilterKernel kernel = blaze::generate(kernelHeight, kernelWidth, [&normalDistribution, &randomEngine](size_t i, size_t j) {
									return normalDistribution(randomEngine);
								});

	size_t rows = 2160;
	size_t columns = 1920;
	auto conv = Conv(rows, columns, kernel.columns(), kernel.rows());


	Conv::Image image;
	for (auto c = 0; c < 3; ++c) {
		Conv::Channel channel = blaze::generate(rows, columns, [&normalDistribution, &randomEngine](size_t i, size_t j) {
			return normalDistribution(randomEngine);
		});
		image[c] = channel;
	}



	using Clock = std::chrono::high_resolution_clock;

	auto t1 = Clock::now();
	auto r = conv(image, kernel);
	auto t2 = Clock::now();
	auto d = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	std::cout << " total convolution() time: " << d.count() << " s" << std::endl;

	//std::cout << image << std::endl;
	//std::cout << kernel << std::endl;
	//std::cout << r << std::endl;
}
