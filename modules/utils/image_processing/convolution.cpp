#include "convolution.hpp"

namespace metric {

	template <typename T, size_t Channels>
	Convolution2d<T, Channels>::Convolution2d(size_t imageWidth, size_t imageHeight, size_t kernelWidth, size_t kernelHeight)
	{
		auto t1 = Clock::now();
		convLayer = std::make_shared<ConvLayer2d>(imageWidth, imageHeight, 1, 1, kernelWidth, kernelHeight);
		auto t2 = Clock::now();
		auto d = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		std::cout << " convolution constructor time: " << d.count() << " s" << std::endl;
	}

	template<typename T, size_t Channels>
	typename Convolution2d<T, Channels>::Image Convolution2d<T, Channels>::operator()(Image image, FilterKernel kernel)
	{
		/* Convert kernel */
		std::vector<T> kernelData(kernel.rows() * kernel.columns());
		size_t e = 0;
		for (size_t i = 0; i < kernel.rows(); ++i) {
			for (size_t j = 0; j < kernel.columns(); ++j) {
				kernelData[e++] = kernel(i, j);
			}
		}

		/* Set kernel */
		auto t1 = Clock::now();
		convLayer->setParameters({kernelData, {0}});
		auto t2 = Clock::now();
		auto d = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		std::cout << " convolution() set kernel time: " << d.count() << " s" << std::endl;


		/* Create output image */
		Image output;

		for (size_t c = 0; c < image.size(); ++c) {
			auto& channel = image[c];

			/* Convert image */
			Matrix imageData(1, channel.rows() * channel.columns());
			e = 0;
			for (size_t i = 0; i < channel.rows(); ++i) {
				for (size_t j = 0; j < channel.columns(); ++j) {
					imageData(0, e++) = channel(i, j);
				}
			}


			auto t1 = Clock::now();
			/* Process */
			convLayer->forward(imageData);
			auto outputData = convLayer->output();

			auto t2 = Clock::now();
			auto d = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
			std::cout << " convolution() time: " << d.count() << " s" << std::endl;

			/* Convert output */
			e = 0;
			auto outputChannel = Channel(convLayer->getOutputShape()[1], convLayer->getOutputShape()[0]);
			for (size_t i = 0; i < outputChannel.rows(); ++i) {
				for (size_t j = 0; j < outputChannel.columns(); ++j) {
					outputChannel(i, j) = outputData(0, e++);
				}
			}

			output[c] = outputChannel;
		}

		return output;
	}
}
