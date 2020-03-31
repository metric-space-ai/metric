#ifndef CONVOLUTION_HPP
#define CONVOLUTION_HPP

#include "../dnn/Activation/Identity.h"
#include "../dnn/Layer/Conv2d.h"

namespace metric {

	template <typename T, size_t Channels>
	class Convolution2d {
		public:
			using Channel = blaze::DynamicMatrix<T>;
			using Image = blaze::StaticVector<Channel, Channels>;
			using FilterKernel = blaze::DynamicMatrix<T>;

			using ConvLayer2d = dnn::Conv2d<T, dnn::Identity<T>>;
			using Matrix = typename ConvLayer2d::Matrix;

			Convolution2d(size_t imageWidth, size_t imageHeight, size_t kernelWidth, size_t kernelHeight);

			Image operator()(Image image, FilterKernel kernel);

			Matrix operator()(Matrix image, std::vector<T> kernelData);


		private:
			std::unique_ptr<ConvLayer2d> convLayer;
	};
}


#include "convolution.cpp"

#endif
