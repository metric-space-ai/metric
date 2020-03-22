#ifndef LAYER_CONV2D_TRANSPOSE_H_
#define LAYER_CONV2D_TRANSPOSE_H_

#include <stdexcept>
#include <chrono>

#include "Conv2d.h"


namespace metric::dnn
{
	template <typename Scalar, typename Activation>
	class Conv2dTranspose: public Conv2d<Scalar, Activation> {
		protected:
			using typename Conv2d<Scalar, Activation>::SparseMatrix;

		public:
			Conv2dTranspose(const int inputWidth, const int inputHeight,
							   const int inputChannels, const int outputChannels,
							   const int kernelWidth, const int kernelHeight,
							   const size_t stride = 1) :
							   Conv2d<Scalar, Activation>(inputWidth, inputHeight,
															inputChannels, outputChannels,
															kernelWidth, kernelHeight,
															stride)

			{
				this->outputWidth = (inputWidth - 1) * stride + kernelWidth;
				this->outputHeight = (inputHeight  - 1) * stride + kernelWidth;

				this->outputSize = this->outputChannels * this->outputWidth * this->outputHeight;
				this->isTranspose = true;
			}

			Conv2dTranspose(const nlohmann::json& json) : Conv2d<Scalar, Activation>(json)
			{
				this->outputWidth = (this->inputWidth  - 1) * this->stride + this->kernelWidth;
				this->outputHeight = (this->inputHeight  - 1) * this->stride + this->kernelHeight;

				this->outputSize = this->outputChannels * this->outputWidth * this->outputHeight;
				this->isTranspose = true;
			}
	};


} // namespace metric:dnn


#endif /* LAYER_CONVOLUTIONAL_H_ */
