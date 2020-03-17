#ifndef LAYER_CONV2D_TRANSPOSE_H_
#define LAYER_CONV2D_TRANSPOSE_H_

#include <stdexcept>
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
class Conv2dTranspose: public Layer<Scalar>
{
    private:
		using Matrix = blaze::DynamicMatrix<Scalar>;
		using Vector = blaze::DynamicVector<Scalar, blaze::rowVector>;
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

	std::vector<size_t> jDeltas;
		std::vector<size_t> iDeltas;

        Vector kernelData;           // Filter parameters. Total length is
        // (in_channels x out_channels x filter_rows x filter_cols)
        // See Utils/Convolution.h for its layout

        Vector m_df_data;               // Derivative of filters, same dimension as m_filter_data

        Vector bias;                  // Bias term for the output channels, out_channels x 1. (One bias term per channel)
        Vector m_db;                    // Derivative of bias, same dimension as m_bias

        Matrix m_z;                     // Linear term, z = conv(in, w) + b. Each column is an observation
        Matrix m_a;                     // Output of this layer, a = act(z)
        Matrix m_din;                   // Derivative of the input of this layer
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
        Conv2dTranspose(const int inputWidth, const int inputHeight,
                      const int inputChannels, const int outputChannels,
                      const int kernelWidth, const int kernelHeight) :
											Layer<Scalar>(inputWidth * inputHeight * inputChannels,
											  (inputWidth + kernelWidth - 1) * (inputHeight + kernelHeight - 1) * outputChannels),
											  inputWidth(inputWidth), inputHeight(inputHeight),
											  kernelWidth(kernelWidth), kernelHeight(kernelHeight),
											  outputWidth(inputWidth + kernelWidth - 1),
											  outputHeight(inputHeight + kernelHeight - 1),
												inputChannels(inputChannels), outputChannels(outputChannels)
	{
		kernelData.resize(inputChannels * outputChannels * kernelWidth * kernelHeight);

	        bias.resize(outputChannels);
        }

        void init(const Scalar& mu, const Scalar& sigma, std::mt19937& rng)
        {
            // Set data dimension
            const int filter_data_size = inputChannels * outputChannels *
										 kernelWidth * kernelHeight;
            kernelData.resize(filter_data_size);
            m_df_data.resize(filter_data_size);
            // Random initialization of filter parameters
            internal::set_normal_random(kernelData.data(), filter_data_size, rng, mu,
                                        sigma);
            // Bias term
            bias.resize(outputChannels);
            m_db.resize(outputChannels);
            internal::set_normal_random(bias.data(), outputChannels, rng, mu, sigma);
        }

        Matrix getUnrolledKernel()
        {
            size_t fromWidth = inputWidth + kernelWidth - 1;
            size_t fromHeight = inputHeight + kernelHeight - 1;

            size_t toWidth = inputWidth;
            size_t toHeight = inputHeight;

			Matrix unrolledKernel(toWidth * toHeight, fromWidth * fromHeight, 0);

			size_t dr = fromWidth - kernelWidth;

			jDeltas.resize(unrolledKernel.rows());
			iDeltas.resize(kernelData.size());

			Vector kernelRow(kernelWidth * kernelHeight + (kernelHeight - 1) * dr);
			kernelRow = 0;

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
			for (size_t i = 0; i < unrolledKernel.rows(); ++i) {
				for (size_t j = 0; j < kernelRow.size(); ++j) {
					unrolledKernel(i, (j00 * fromWidth) + j0 + j) = kernelRow[j];
				}

				jDeltas[i] = (j00 * fromWidth) + j0;

				if (j0 + 1 + kernelWidth > fromWidth) {
					j0  = 0;
					++j00;
				} else {
					++j0;
				}
			}

			return unrolledKernel;
        }


        void forward(const Matrix& prev_layer_data)
        {
            // Each column is an observation
            const int nobs = prev_layer_data.columns();

	        // Linear term, z = conv(in, w) + b
            m_z.resize(this->outputSize, nobs);

	        // Convolution
	        auto unrolledKernel = getUnrolledKernel();
	        blaze::transpose(unrolledKernel);

	        //cout << prev_layer_data;
	        //cout << getUnrolledKernel() << endl;
	        //cout << m_filter_data << endl;
	        //cout << unrolledKernel << endl;
	        for (int i = 0; i < nobs; ++i) {
		        blaze::column(m_z, i) = unrolledKernel * blaze::column(prev_layer_data, i);
	        }


	        // Add bias terms
            // Each column of m_z contains outputChannels channels, and each channel has
            // m_dim.conv_rows * m_dim.conv_cols elements
            int channel_start_row = 0;
            const int channel_nelem = outputHeight * outputWidth;

            for (int i = 0; i < outputChannels; i++, channel_start_row += channel_nelem)
            {
                submatrix(m_z, channel_start_row, 0, channel_nelem, nobs) += bias[i];
            }


	        /* Apply activation function */
            m_a.resize(this->outputSize, nobs);
            Activation::activate(m_z, m_a);
        }

        const Matrix& output() const
        {
            return m_a;
        }

        // prev_layer_data: getInputSize x nobs
        // next_layer_data: getOutputSize x nobs
        // https://grzegorzgwardys.wordpress.com/2016/04/22/8/
        void backprop(const Matrix& prev_layer_data, const Matrix& next_layer_data)
        {
            const int nobs = prev_layer_data.columns();
            Matrix& dLz = m_z;
            Activation::apply_jacobian(m_z, m_a, next_layer_data, dLz);

            /* Derivative for weights */
            Matrix kernelDerivatives(kernelData.size(), nobs, 0);
            for (size_t i = 0; i < nobs; ++i) {
	            Matrix W(outputWidth * outputHeight, inputWidth * inputHeight);
	            for (size_t k = 0; k < dLz.rows(); ++k) {
		            blaze::row(W, k) = blaze::trans(blaze::column(prev_layer_data, i) * dLz(k, i));
	            }
	            blaze::transpose(W);
	            for (size_t j = 0; j < kernelData.size(); ++j) {
		            for (size_t k = 0; k < jDeltas.size(); ++k) {
		            	kernelDerivatives(j, i) += W(k, jDeltas[k] + iDeltas[j]);
		            }
	            }
            }

            m_df_data = blaze::mean<blaze::columnwise>(kernelDerivatives);


            /* Derivative for bias */
            ConstAlignedMapMat dLz_by_channel(dLz.data(), outputWidth * outputHeight,
														  outputChannels * nobs);
            Vector dLb = blaze::trans(blaze::sum<blaze::rowwise>(dLz_by_channel));

            // Average over observations
            ConstAlignedMapMat dLb_by_obs(dLb.data(), outputChannels, nobs);
            m_db = blaze::mean<blaze::columnwise>(dLb_by_obs);


            m_din.resize(this->inputSize, nobs);

	        auto unrolledKernel = getUnrolledKernel();

	        for (int i = 0; i < nobs; ++i) {
		        blaze::column(m_din, i) = unrolledKernel * blaze::column(dLz, i);
	        }
        }

        const Matrix& backprop_data() const
        {
            return m_din;
        }

        void update(Optimizer<Scalar>& opt)
        {
            ConstAlignedMapVec dw(m_df_data.data(), m_df_data.size());
            ConstAlignedMapVec db(m_db.data(), m_db.size());
            AlignedMapVec      w(kernelData.data(), kernelData.size());
            AlignedMapVec      b(bias.data(), bias.size());
            opt.update(dw, w);
            opt.update(db, b);
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
			std::vector<Scalar> res(m_df_data.size() + m_db.size());
            // Copy the data of filters and bias to a long vector
            std::copy(m_df_data.data(), m_df_data.data() + m_df_data.size(), res.begin());
            std::copy(m_db.data(), m_db.data() + m_db.size(),
                      res.begin() + m_df_data.size());
            return res;
        }
};


} // namespace dnn
} // namespace metric


#endif /* LAYER_CONV2D_TRANSPOSE_H_ */
