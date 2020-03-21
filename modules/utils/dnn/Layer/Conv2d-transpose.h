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

				this->kernelsData.resize(inputChannels * outputChannels * kernelWidth * kernelHeight);

				this->bias.resize(outputChannels);
			}

			Conv2dTranspose(const nlohmann::json& json) : Conv2d<Scalar, Activation>(json)
			{
				this->outputWidth = (this->inputWidth  - 1) * this->stride + this->kernelWidth;
				this->outputHeight = (this->inputHeight  - 1) * this->stride + this->kernelHeight;

				this->kernelsData.resize(this->inputChannels * this->outputChannels * this->kernelWidth * this->kernelHeight);

				this->bias.resize(this->outputChannels);

				this->outputSize = this->outputChannels * this->outputWidth * this->outputHeight;
			}

			void getUnrolledKernel()
			{
				Conv2d<Scalar, Activation>::getUnrolledKernel();

				std::vector<SparseMatrix> kernels;
				for (const auto kernel: this->unrolledKernels) {
					//std::cout << kernel << std::endl;
					blaze::trans(kernel);
				}
			}

		void init(const Scalar& mu, const Scalar& sigma, std::mt19937& rng)
		{
			// Set data dimension
			const int kernelDataSize = this->inputChannels * this->outputChannels * this->kernelWidth * this->kernelHeight;

			this->kernelsData.resize(kernelDataSize);
			this->df_data.resize(kernelDataSize);

			// Random initialization of filter parameters
			internal::set_normal_random(this->kernelsData.data(), kernelDataSize, rng, mu, sigma);

			getUnrolledKernel();

			// Bias term
			this->bias.resize(this->outputChannels);
			this->db.resize(this->outputChannels);

			internal::set_normal_random(this->bias.data(), this->outputChannels, rng, mu, sigma);
		}
		void update(Optimizer<Scalar>& opt)
		{
			/*ConstAlignedMapVec dw(df_data.data(), df_data.size());
			ConstAlignedMapVec dbConst(db.data(), db.size());
			AlignedMapVec      w(kernelsData.data(), kernelsData.size());
			AlignedMapVec      b(bias.data(), bias.size());*/

				opt.update(this->df_data, this->kernelsData);
				opt.update(this->db, this->bias);

				getUnrolledKernel();
			}
	};


} // namespace metric:dnn


#endif /* LAYER_CONVOLUTIONAL_H_ */
