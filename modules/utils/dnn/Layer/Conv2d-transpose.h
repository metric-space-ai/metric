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
                                                            Conv2d<Scalar, Activation>(inputWidth * inputHeight * inputChannels,
                                                                          ((inputWidth - kernelWidth) / stride + 1) *
                                                                          ((inputHeight - kernelHeight) / stride + 1) * outputChannels)
                        {
                            this->inputWidth = inputWidth;
                            this->inputHeight = inputHeight;
                            this->kernelWidth = kernelWidth;
                            this->kernelHeight = kernelHeight;
                            this->inputChannels = inputChannels;
                            this->outputChannels = outputChannels;
                            this->stride = stride;

                            this->outputWidth = (inputWidth - 1) * stride + kernelWidth;
                            this->outputHeight = (inputHeight  - 1) * stride + kernelWidth;

                            this->inputSize = inputChannels * inputWidth * inputHeight;
                            this->outputSize = this->outputChannels * this->outputWidth * this->outputHeight;

                            // Set data dimension
                            const int kernelDataSize = inputChannels * outputChannels * kernelWidth * kernelHeight;

                            this->kernelsData.resize(kernelDataSize);
                            this->df_data.resize(kernelDataSize);

                            // Bias term
                            this->bias.resize(outputChannels);
                            this->db.resize(outputChannels);

                            this->isTranspose = true;

                            this->calculateUnrolledKernelStructure();
                            this->getUnrolledKernel();

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
