#include "convolution.hpp"


namespace metric {

	template <typename T, size_t Channels>
	Convolution2d<T, Channels>::Convolution2d(size_t imageWidth, size_t imageHeight,
											   size_t kernelWidth, size_t kernelHeight) :
																		padWidth(kernelWidth - 1),
																		padHeight(kernelHeight - 1)
	{
		padModel = std::make_shared<PadModel<T>>(PadDirection::BOTH, PadType::CONST, 0);

		convLayer = std::make_shared<ConvLayer2d>(imageWidth + padWidth, imageHeight + padHeight, 1, 1, kernelWidth, kernelHeight);
	}

	template<typename T, size_t Channels>
	void Convolution2d<T, Channels>::setKernel(const FilterKernel& kernel)
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
		convLayer->setParameters({kernelData, {0}});
	}

	template<typename T, size_t Channels>
	typename Convolution2d<T, Channels>::Image Convolution2d<T, Channels>::operator()(const Image& image)
	{
		/* Create output image */
		Image output;

		for (size_t c = 0; c < image.size(); ++c) {
			const auto& channel = padModel->pad({padWidth / 2, padHeight / 2}, image[c]).first;

			/* Convert image */
			Matrix imageData(1, channel.rows() * channel.columns());
			size_t e = 0;
			for (size_t i = 0; i < channel.rows(); ++i) {
				for (size_t j = 0; j < channel.columns(); ++j) {
					imageData(0, e++) = channel(i, j);
				}
			}


			/* Process */
			convLayer->forward(imageData);
			auto outputData = convLayer->output();


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

	template<typename T, size_t Channels>
	typename Convolution2d<T, Channels>::Image Convolution2d<T, Channels>::operator()(const Image& image,
																						const FilterKernel& kernel)
	{
		setKernel(kernel);
		return operator()(image);
	}
}
