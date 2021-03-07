#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include <iostream>
#include "modules/utils/image_processing/convolution.hpp"


using namespace metric;

using T = double;
using Conv = Convolution2d<T, 1>;

std::tuple<Conv::Image, Conv::FilterKernel> generateImageAndKernel(size_t imageWidth, size_t imageHeight,
																   size_t kernelWidth, size_t kernelHeight)
{
		std::normal_distribution<T> normalDistribution(0.5, 0.25);
		std::mt19937 randomEngine{std::random_device()()};

		Conv::FilterKernel kernel = blaze::generate(kernelHeight, kernelWidth,
		                                            [&normalDistribution, &randomEngine](size_t i, size_t j) {
			                                            return normalDistribution(randomEngine);
		                                            });


		Conv::Image image;
		for (auto& channel: image) {
			channel = blaze::generate(imageHeight, imageWidth,
			                                        [&normalDistribution, &randomEngine](size_t i, size_t j) {
				                                        return normalDistribution(randomEngine);
			                                        });
		}

		return {image, kernel};
}

TEST_CASE("Convolution2d benchmarks")
{
	/*auto [imageWidth, imageHeight] = GENERATE(table<size_t, size_t>({{320, 240},
															{640, 480},
															 {1920, 1080}}));*/
	size_t imageWidth = 1920;
	size_t imageHeight = 1080;

//auto kernelSize = GENERATE(3, 5, 7);
	size_t kernelSize = 5;

	size_t kernelWidth = kernelSize;
	size_t kernelHeight = kernelSize;


	/*BENCHMARK("Convolution2d()")
	{
		return Conv(imageWidth, imageHeight, kernelWidth, kernelHeight);
	};


	auto [image, kernel] = generateImageAndKernel(imageWidth, imageHeight, kernelWidth, kernelHeight);
	auto conv = Conv(imageWidth, imageHeight, kernelWidth, kernelHeight);
	return conv;

	BENCHMARK("Convolution2d.setKernel()")
	{
		return conv.setKernel(kernel);
	};

	*/
	auto [image, kernel] = generateImageAndKernel(imageWidth, imageHeight, kernelWidth, kernelHeight);
	auto conv = Conv(imageWidth, imageHeight, kernelWidth, kernelHeight);
	conv.setKernel(kernel);

	BENCHMARK("Convolution2d.operator()")
	{
		return conv(image);
	};
}
