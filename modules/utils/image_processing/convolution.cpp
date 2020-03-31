#include "convolution.hpp"

namespace metric {

	template <typename T, size_t Channels>
	Convolution2d<T, Channels>::Convolution2d(size_t imageWidth, size_t imageHeight, size_t kernelWidth, size_t kernelHeight)
	{
		convLayer = std::make_unique<ConvLayer2d>(imageWidth, imageHeight, Channels, Channels, kernelWidth, kernelHeight);
	}

	template<typename T, size_t Channels>
	typename Convolution2d<T, Channels>::Image Convolution2d<T, Channels>::operator()(Image image, FilterKernel kernel)
	{
		/* Convert image */
		Matrix imageData(image.rows() * image.columns() * Channels);
		size_t e = 0;
		for (size_t c = 0; c < image.size(); ++c) {
			auto& imageChannel = image[c];
			for (size_t i = 0; i < imageChannel.rows(); ++i) {
				for (size_t j = 0; j < imageChannel.columns(); ++j) {
					imageData(0, e++) = imageChannel(i, j);
				}
			}
		}


		/* Convert kernel */
		std::vector<T> kernelData(kernel.rows() * kernel.columns() * Channels);
		e = 0;
		for (size_t c = 0; c < image.size(); ++c) {
			for (size_t i = 0; i < kernel.rows(); ++i) {
				for (size_t j = 0; j < kernel.columns(); ++j) {
					kernelData[e++] = kernel(i, j);
				}
			}
		}

		/* Process */
		auto outputData = operator()(imageData, kernelData);

		/* Convert output */
		Image output;

		e = 0;
		for (size_t c = 0; c < output.size(); ++c) {
			auto channel = Channel(convLayer->getOuputHeight(), convLayer->getOutputWidth());
			for (size_t i = 0; i < channel.rows(); ++i) {
				for (size_t j = 0; j < channel.columns(); ++j) {
					channel(i, j) = outputData(0, e++);
				}
			}
			output[c] = channel;
		}

		return output;
	}

	template <typename T, size_t Channels>
	typename Convolution2d<T, Channels>::Matrix Convolution2d<T, Channels>::operator()(Matrix image, std::vector<T> kernelData)
	{
		convLayer->setParameters(kernelData, {});

		convLayer->forward(image);

		return convLayer->output();
	}
}
