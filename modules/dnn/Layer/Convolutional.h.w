#ifndef LAYER_CONVOLUTIONAL_H_
#define LAYER_CONVOLUTIONAL_H_

#include <stdexcept>
#include "../Layer.h"
#include "../Utils/Convolution.h"
#include "../Utils/Random.h"

using namespace std;

namespace MiniDNN
{


///
/// \ingroup Layers
///
/// Convolutional hidden layer
///
/// Currently only supports the "valid" rule of convolution.
///
template <typename Scalar, typename Activation>
class Convolutional: public Layer<Scalar>
{
    private:
		using Matrix = blaze::DynamicMatrix<Scalar, blaze::columnMajor>;
		using Vector = blaze::DynamicVector<Scalar>;
		using ConstAlignedMapMat = const blaze::CustomMatrix<Scalar, blaze::unaligned, blaze::unpadded, blaze::columnMajor>;
		using ConstAlignedMapVec = const blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;
		using AlignedMapVec = blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;

		const internal::ConvDims m_dim; // Various dimensions of convolution

		size_t inputWidth;
		size_t inputHeight;
		size_t kernelWidth;
		size_t kernelHeight;
		size_t outputWidth;
		size_t outputHeight;

	Vector m_filter_data;           // Filter parameters. Total length is
        // (in_channels x out_channels x filter_rows x filter_cols)
        // See Utils/Convolution.h for its layout

        Vector m_df_data;               // Derivative of filters, same dimension as m_filter_data

        Vector m_bias;                  // Bias term for the output channels, out_channels x 1. (One bias term per channel)
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
        Convolutional(const int inputWidth, const int inputHeight,
                      const int inputChannels, const int outputChannels,
                      const int kernelWidth, const int kernelHeight) :
							Layer<Scalar>(inputWidth * inputHeight * inputChannels,
										  (inputWidth - kernelWidth + 1) * (inputHeight - kernelHeight + 1) * outputChannels),
							m_dim(inputChannels, outputChannels, inputHeight, inputWidth, kernelHeight,
									  kernelWidth),
									  inputWidth(inputWidth), inputHeight(inputHeight),
									  kernelWidth(kernelWidth), kernelHeight(kernelHeight),
									  outputWidth(inputWidth - kernelWidth + 1),
									  outputHeight(inputHeight - kernelHeight + 1)
	{
		const int filter_data_size = m_dim.in_channels * m_dim.out_channels *
	                                     m_dim.filter_rows * m_dim.filter_cols;
	        m_filter_data.resize(filter_data_size);

	        m_bias.resize(m_dim.out_channels);
        }

        void init(const Scalar& mu, const Scalar& sigma, RNG& rng)
        {
            // Set data dimension
            const int filter_data_size = m_dim.in_channels * m_dim.out_channels *
                                         m_dim.filter_rows * m_dim.filter_cols;
            m_filter_data.resize(filter_data_size);
            m_df_data.resize(filter_data_size);
            // Random initialization of filter parameters
            internal::set_normal_random(m_filter_data.data(), filter_data_size, rng, mu,
                                        sigma);
            // Bias term
            m_bias.resize(m_dim.out_channels);
            m_db.resize(m_dim.out_channels);
            internal::set_normal_random(m_bias.data(), m_dim.out_channels, rng, mu, sigma);
        }

        Matrix getW(const Vector& data)
        {
			Matrix W(outputWidth * outputHeight, m_filter_data.size());

			for (size_t i = 0; i < outputHeight; ++i ) {
				for (size_t j = 0; j < outputWidth; ++j) {
					for (size_t ii = 0; ii < kernelHeight; ++ii) {
						for (size_t jj = 0; jj < kernelWidth; ++jj) {
							auto iW = i * outputWidth + j;
							auto jW = ii * kernelWidth + jj;

							W(iW, jW) = data[(i + ii) * inputWidth + (j + jj)];
						}
					}
				}
			}



			return W;
        }

        void forward(const Matrix& prev_layer_data)
        {
            // Each column is an observation
            const int nobs = prev_layer_data.columns();

	        // Linear term, z = conv(in, w) + b
            m_z.resize(this->m_out_size, nobs);

            for (int i = 0; i < nobs; ++i) {
	            // Convolution
	            auto W = getW(blaze::column(prev_layer_data, i));

	            //cout << prev_layer_data;
	            //cout << W << endl;
	            //cout << m_filter_data << endl;
	            blaze::column(m_z, i) = W * m_filter_data;
            }

	        // Add bias terms
            // Each column of m_z contains m_dim.out_channels channels, and each channel has
            // m_dim.conv_rows * m_dim.conv_cols elements
            int channel_start_row = 0;
            const int channel_nelem = m_dim.conv_rows * m_dim.conv_cols;

            for (int i = 0; i < m_dim.out_channels; i++, channel_start_row += channel_nelem)
            {
                submatrix(m_z, channel_start_row, 0, channel_nelem, nobs) += m_bias[i];
            }


	        /* Apply activation function */
            m_a.resize(this->m_out_size, nobs);
            Activation::activate(m_z, m_a);
        }

        const Matrix& output() const
        {
            return m_a;
        }

        // prev_layer_data: in_size x nobs
        // next_layer_data: out_size x nobs
        // https://grzegorzgwardys.wordpress.com/2016/04/22/8/
        void backprop(const Matrix& prev_layer_data, const Matrix& next_layer_data)
        {
            const int nobs = prev_layer_data.columns();
            // After forward stage, m_z contains z = conv(in, w) + b
            // Now we need to calculate d(L) / d(z) = [d(a) / d(z)] * [d(L) / d(a)]
            // d(L) / d(a) is computed in the next layer, contained in next_layer_data
            // The Jacobian matrix J = d(a) / d(z) is determined by the activation function
            Matrix& dLz = m_z;
            Activation::apply_jacobian(m_z, m_a, next_layer_data, dLz);
            // z_j = sum_i(conv(in_i, w_ij)) + b_j
            //
            // d(z_k) / d(w_ij) = 0, if k != j
            // d(L) / d(w_ij) = [d(z_j) / d(w_ij)] * [d(L) / d(z_j)] = sum_i{ [d(z_j) / d(w_ij)] * [d(L) / d(z_j)] }
            // = sum_i(conv(in_i, d(L) / d(z_j)))
            //
            // z_j is an image (matrix), b_j is a scalar
            // d(z_j) / d(b_j) = a matrix of the same size of d(z_j) filled with 1
            // d(L) / d(b_j) = (d(L) / d(z_j)).sum()
            //
            // d(z_j) / d(in_i) = conv_full_op(w_ij_rotate)
            // d(L) / d(in_i) = sum_j((d(z_j) / d(in_i)) * (d(L) / d(z_j))) = sum_j(conv_full(d(L) / d(z_j), w_ij_rotate))
            // Derivative for weights
            internal::ConvDims back_conv_dim(nobs, m_dim.out_channels, m_dim.channel_rows,
                                             m_dim.channel_cols,
                                             m_dim.conv_rows, m_dim.conv_cols);
            internal::convolve_valid(back_conv_dim, prev_layer_data.data(), false,
                                     m_dim.in_channels,
                                     dLz.data(), m_df_data.data()
                                    );
            m_df_data /= nobs;
            // Derivative for bias
            // Aggregate d(L) / d(z) in each output channel
            ConstAlignedMapMat dLz_by_channel(dLz.data(), outputWidth * outputHeight,
                                              m_dim.out_channels * nobs);
            Vector dLb = blaze::trans(blaze::sum<blaze::columnwise>(dLz_by_channel));
            // Average over observations
            /* noalias */
            ConstAlignedMapMat dLb_by_obs(dLb.data(), m_dim.out_channels, nobs);
            m_db = blaze::mean<blaze::rowwise>(dLb_by_obs);
            // Compute d(L) / d_in = conv_full(d(L) / d(z), w_rotate)
            m_din.resize(this->m_in_size, nobs);
            internal::ConvDims conv_full_dim(m_dim.out_channels, m_dim.in_channels,
                                             m_dim.conv_rows, m_dim.conv_cols, m_dim.filter_rows, m_dim.filter_cols);
            internal::convolve_full(conv_full_dim, dLz.data(), nobs,
                                    m_filter_data.data(), m_din.data()
                                   );
        }

        const Matrix& backprop_data() const
        {
            return m_din;
        }

        void update(Optimizer<Scalar>& opt)
        {
            ConstAlignedMapVec dw(m_df_data.data(), m_df_data.size());
            ConstAlignedMapVec db(m_db.data(), m_db.size());
            AlignedMapVec      w(m_filter_data.data(), m_filter_data.size());
            AlignedMapVec      b(m_bias.data(), m_bias.size());
            opt.update(dw, w);
            opt.update(db, b);
        }

        std::vector<Scalar> get_parameters() const
        {
            std::vector<Scalar> res(m_filter_data.size() + m_bias.size());
            // Copy the data of filters and bias to a long vector
            std::copy(m_filter_data.data(), m_filter_data.data() + m_filter_data.size(),
                      res.begin());
            std::copy(m_bias.data(), m_bias.data() + m_bias.size(),
                      res.begin() + m_filter_data.size());
            return res;
        }

        void setParameters(const Vector &kernelData, const Vector &biasData)
        {
            if (kernelData.size() != m_filter_data.size()) {
                throw std::invalid_argument("Parameter size does not match");
            }

			if (biasData.size() != m_bias.size()) {
				throw std::invalid_argument("Parameter size does not match");
			}

			m_filter_data = kernelData;
			m_bias = biasData;
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


} // namespace MiniDNN


#endif /* LAYER_CONVOLUTIONAL_H_ */
