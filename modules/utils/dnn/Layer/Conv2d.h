#ifndef LAYER_CONVOLUTIONAL_H_
#define LAYER_CONVOLUTIONAL_H_

#include <stdexcept>
#include <chrono>

#include "../Layer.h"
#include "../Utils/Convolution.h"
#include "../Utils/Random.h"


namespace metric
{
namespace dnn
{


///
/// \ingroup Layers
///
/// Conv2dTranspose hidden layer
///
/// Currently only supports the "valid" rule of convolution.
///
template <typename Scalar, typename Activation>
class Conv2d: public Layer<Scalar>
{
    private:
		using Matrix = blaze::DynamicMatrix<Scalar>;
		using SparceMatrix = blaze::CompressedMatrix<Scalar, blaze::columnMajor>;
		using ColumnMatrix = blaze::DynamicMatrix<Scalar, blaze::columnMajor>;
		using Vector = blaze::DynamicVector<Scalar, blaze::rowVector>;
		using SparceVector = blaze::CompressedVector<Scalar, blaze::rowVector>;
		using ConstAlignedMapMat = const blaze::CustomMatrix<Scalar, blaze::unaligned, blaze::unpadded>;
		using ConstAlignedMapVec = const blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;
		using AlignedMapVec = blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;


		size_t inputWidth;
		size_t inputHeight;
		size_t kernelWidth;
		size_t kernelHeight;
		size_t outputWidth;
		size_t outputHeight;
		size_t inputChannels;
		size_t outputChannels;

		SparceMatrix unrolledKernel;
		std::vector<size_t> jDeltas;
		std::vector<size_t> iDeltas;

        Vector kernelData;           // Filter parameters. Total length is
        // (in_channels x out_channels x filter_rows x filter_cols)
        // See Utils/Convolution.h for its layout

        Vector df_data;               // Derivative of filters, same dimension as m_filter_data

        Vector bias;                  // Bias term for the output channels, out_channels x 1. (One bias term per channel)
        Vector db;                    // Derivative of bias, same dimension as m_bias

        Matrix z;                     // Linear term, z = conv(in, w) + b. Each column is an observation
        Matrix a;                     // Output of this layer, a = act(z)
        Matrix din;                   // Derivative of the input of this layer
        // Note that input of this layer is also the output of previous layer

    public:
        ///
        /// Constructor
        ///
        /// \param inputWidth      Width of the input image in each channel.
        /// \param inputHeight     Height of the input image in each channel.
        /// \param inputChannels   Number of input channels.
        /// \param outputChannels  Number of output channels.
        /// \param kernelWidth  Width of the filter.
        /// \param kernelHeight Height of the filter.
        ///
        Conv2d(const int inputWidth, const int inputHeight,
               const int inputChannels, const int outputChannels,
               const int kernelWidth, const int kernelHeight) :
											Layer<Scalar>(inputWidth * inputHeight * inputChannels,
											  (inputWidth - kernelWidth + 1) * (inputHeight - kernelHeight + 1) * outputChannels),
											  inputWidth(inputWidth), inputHeight(inputHeight),
											  kernelWidth(kernelWidth), kernelHeight(kernelHeight),
											  outputWidth(inputWidth - kernelWidth + 1),
											  outputHeight(inputHeight - kernelHeight + 1),
											  inputChannels(inputChannels), outputChannels(outputChannels)
        {
	        kernelData.resize(inputChannels * outputChannels * kernelWidth * kernelHeight);

	        bias.resize(outputChannels);
        }

        void init(const Scalar& mu, const Scalar& sigma, std::mt19937& rng)
        {
            // Set data dimension
            const int kernelDataSize = inputChannels * outputChannels * kernelWidth * kernelHeight;

            kernelData.resize(kernelDataSize);
            df_data.resize(kernelDataSize);

            // Random initialization of filter parameters
            internal::set_normal_random(kernelData.data(), kernelDataSize, rng, mu, sigma);

            // Bias term
            bias.resize(outputChannels);
            db.resize(outputChannels);

            internal::set_normal_random(bias.data(), outputChannels, rng, mu, sigma);
        }

        void getUnrolledKernel()
        {
            const size_t fromWidth = inputWidth;
            const size_t fromHeight = inputHeight;

            const size_t toWidth = outputWidth;
            const size_t toHeight = outputHeight;

			unrolledKernel.resize(fromWidth * fromHeight, toWidth * toHeight);
			blaze::reset(unrolledKernel);

			const size_t dr = fromWidth - kernelWidth;

			jDeltas.resize(unrolledKernel.columns());
			iDeltas.resize(kernelData.size());

			SparceVector kernelRow(kernelWidth * kernelHeight + (kernelHeight - 1) * dr);
			//kernelRow = 0;

			size_t p = 0;
			for (size_t i = 0; i < kernelData.size(); ++i) {
				kernelRow[p++] = kernelData[i];
				iDeltas[i] = p;

				if ((i + 1) % kernelWidth == 0) {
					p += dr;
				}
			}

			size_t j00 = 0;
	        size_t j0 = 0;
			for (size_t i = 0; i < unrolledKernel.columns(); ++i) {
				for (size_t j = 0; j < kernelRow.size(); ++j) {
					unrolledKernel((j00 * fromWidth) + j0 + j, i) = kernelRow[j];
				}

				jDeltas[i] = (j00 * fromWidth) + j0;

				if (j0 + 1 + kernelWidth > fromWidth) {
					j0  = 0;
					++j00;
				} else {
					++j0;
				}
			}
        }


        void forward(const Matrix& prev_layer_data)
        {
            // Each column is an observation
            const int nobs = prev_layer_data.rows();

	        // Linear term, z = conv(in, w) + b
            z.resize(nobs, this->outputSize);

	        // Convolution
	        getUnrolledKernel();

	        //cout << prev_layer_data;
	        //cout << getUnrolledKernel() << endl;
	        //cout << m_filter_data << endl;
	        //cout << unrolledKernel << endl;
	        for (int i = 0; i < nobs; ++i) {
		        blaze::row(z, i) = blaze::row(prev_layer_data, i) * unrolledKernel;
	        }


	        // Add bias terms
            // Each column of z contains m_dim.out_channels channels, and each channel has
            // m_dim.conv_rows * m_dim.conv_cols elements
            int channel_start_row = 0;
            const int channel_nelem = outputWidth * outputHeight;
	        for (int i = 0; i < outputChannels; i++, channel_start_row += channel_nelem) {
		        submatrix(z, 0, channel_start_row, nobs, channel_nelem) += bias[i];
	        }


	        /* Apply activation function */
            a.resize(nobs, this->outputSize);
            Activation::activate(z, a);
        }

        const Matrix& output() const
        {
            return a;
        }

        // prev_layer_data: getInputSize x nobs
        // next_layer_data: getOutputSize x nobs
        // https://grzegorzgwardys.wordpress.com/2016/04/22/8/
        void backprop(const Matrix& prev_layer_data, const Matrix& next_layer_data)
        {
            const size_t nobs = prev_layer_data.rows();

            //ColumnMatrix dLz = z;
	        Matrix dLz = z;
	        Activation::apply_jacobian(z, a, next_layer_data, dLz);


	        auto t1 = std::chrono::high_resolution_clock::now();
            /* Derivative for weights */
            ColumnMatrix kernelDerivatives(nobs, kernelData.size(), 0);
	        ColumnMatrix W(inputWidth * inputHeight, outputWidth * outputHeight);
	        for (size_t i = 0; i < nobs; ++i) {
	            for (size_t k = 0; k < dLz.columns(); ++k) {
	                blaze::column(W, k) = blaze::trans(blaze::row(prev_layer_data, i) * dLz(i, k));
	            }

	            for (size_t j = 0; j < kernelData.size(); ++j) {
		            for (size_t k = 0; k < jDeltas.size(); ++k) {
		            	kernelDerivatives(i, j) += W(jDeltas[k] + iDeltas[j], k);
		            }
	            }
            }
	        auto t2 = std::chrono::high_resolution_clock::now();
	        auto d = std::chrono::duration_cast < std::chrono::duration < double >> (t2 - t1);
	        std::cout << "time: " << d.count() << " s" << std::endl;

	        /* Average over observations */
	        df_data = blaze::mean<blaze::columnwise>(kernelDerivatives);


            /* Derivative for bias */
            ConstAlignedMapMat dLz_by_channel(dLz.data(), outputChannels * nobs, outputWidth * outputHeight);
            Vector dLb = blaze::trans(blaze::sum<blaze::rowwise>(dLz_by_channel));

	        /* Average over observations */
            ConstAlignedMapMat dLb_by_obs(dLb.data(), nobs, outputChannels);
	        db = blaze::mean<blaze::columnwise>(dLb_by_obs);


	        /* Derivative for input */
	        din.resize(nobs, this->inputSize);
	        for (int i = 0; i < nobs; ++i) {
		        blaze::row(din, i) = blaze::row(dLz, i) * blaze::trans(unrolledKernel);
	        }
        }

        const Matrix& backprop_data() const
        {
            return din;
        }

        void update(Optimizer<Scalar>& opt)
        {
            ConstAlignedMapVec dw(df_data.data(), df_data.size());
            ConstAlignedMapVec dbConst(db.data(), db.size());
            AlignedMapVec      w(kernelData.data(), kernelData.size());
            AlignedMapVec      b(bias.data(), bias.size());
            opt.update(dw, w);
            opt.update(dbConst, b);
        }

		std::vector<std::vector<Scalar>> getParameters() const
		{
			std::vector<std::vector<Scalar>> parameters(2);

            // kernels
            std::copy(kernelData.data(), kernelData.data() + kernelData.size(),
                      parameters[0].begin());
			// bias
            std::copy(bias.data(), bias.data() + bias.size(),
                      parameters[1].begin());

            return parameters;
        }

		void setParameters(const std::vector<std::vector<Scalar>>& parameters)
		{
			/*if (static_cast<int>(param.size()) != blaze::size(m_weight) + m_bias.size())
			{
				throw std::invalid_argument("Parameter size does not match");
			}*/

			std::copy(parameters[0].begin(), parameters[0].begin() + blaze::size(kernelData), kernelData.data());
			std::copy(parameters[1].begin(), parameters[1].begin() + blaze::size(bias), bias.data());
		}

		std::vector<Scalar> get_derivatives() const
		{
			std::vector<Scalar> res(df_data.size() + db.size());
            // Copy the data of filters and bias to a long vector
            std::copy(df_data.data(), df_data.data() + df_data.size(), res.begin());
            std::copy(db.data(), db.data() + db.size(),
                      res.begin() + df_data.size());
            return res;
        }
};


} // namespace dnn
} // namespace metric


#endif /* LAYER_CONVOLUTIONAL_H_ */
