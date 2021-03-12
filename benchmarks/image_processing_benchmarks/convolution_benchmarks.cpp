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
	const auto [imageWidth, imageHeight] = GENERATE(table<size_t, size_t>({/*{320, 240},
															{640, 480},*/
															 {1920, 1080}}));

	const auto kernelSize = GENERATE(5/*3, 5, 7*/);

	const size_t kernelWidth = kernelSize;
	const size_t kernelHeight = kernelSize;


	const std::string imageSizeString = "image: " + std::to_string(imageWidth) + "x" + std::to_string(imageHeight);
	const std::string kernelSizeString = "kernel: " + std::to_string(kernelWidth) + "x" + std::to_string(kernelHeight);
	const std::string postfix = "[" + imageSizeString + " " + kernelSizeString + "]";

	/*BENCHMARK("Convolution2d() " + postfix)
	{
		return Conv(imageWidth, imageHeight, kernelWidth, kernelHeight);
	};



	BENCHMARK_ADVANCED("Convolution2d.setKernel() " + postfix)(Catch::Benchmark::Chronometer meter)
	{
		const auto [image, kernel] = generateImageAndKernel(imageWidth, imageHeight, kernelWidth, kernelHeight);
		auto conv = Conv(imageWidth, imageHeight, kernelWidth, kernelHeight);

		meter.measure([&conv, &kernel] { return conv.setKernel(kernel); });
	};

*/
	BENCHMARK_ADVANCED("Convolution2d.operator() " + postfix)(Catch::Benchmark::Chronometer meter)
	{
		const auto [image, kernel] = generateImageAndKernel(imageWidth, imageHeight, kernelWidth, kernelHeight);
		auto conv = Conv(imageWidth, imageHeight, kernelWidth, kernelHeight);
		conv.setKernel(kernel);

		meter.measure([&conv, &image] { return conv(image); });
	};
}
