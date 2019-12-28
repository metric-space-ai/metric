#ifndef LAYER_CONV2D_TRANSPOSE_H_
#define LAYER_CONV2D_TRANSPOSE_H_

#include <stdexcept>
#include "../Layer.h"
#include "../Utils/Convolution.h"
#include "../Utils/Random.h"


namespace MiniDNN
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
    public:
    
        class LayerSerialProxy : public Layer<Scalar>::LayerSerialProxy {
        public:
            LayerSerialProxy() : Layer<Scalar>::LayerSerialProxy() {}
            LayerSerialProxy(Layer<Scalar>* save_layer) : Layer<Scalar>::LayerSerialProxy(save_layer) {}

            template<class Archive>
            void load(Archive & ar)
            {
                int inputWidth;
                int inputHeight;
                int inputChannels;
                int outputChannels;
                int kernelWidth;
                int kernelHeight;

                ar(CEREAL_NVP(inputWidth),
                   CEREAL_NVP(inputHeight),
                   CEREAL_NVP(inputChannels),
                   CEREAL_NVP(outputChannels),
                   CEREAL_NVP(kernelWidth),
                   CEREAL_NVP(kernelHeight));

                Layer<Scalar>::LayerSerialProxy::load_layer = std::make_shared<Conv2dTranspose<Scalar,
                    Activation>>(inputWidth,
                                 inputHeight,
                                 inputChannels,
                                 outputChannels,
                                 kernelWidth,
                                 kernelHeight);
            }

            template<class Archive>
            void save(Archive & ar) const
            {
                Conv2dTranspose<Scalar, Activation>* sl = dynamic_cast<Conv2dTranspose<Scalar, Activation>*>(Layer<Scalar>::LayerSerialProxy::save_layer);
                int inputWidth = sl->inputWidth;
                int inputHeight = sl->inputHeight;
                int inputChannels = sl->inputChannels;
                int outputChannels = sl->outputChannels;
                int kernelWidth = sl->kernelWidth;
                int kernelHeight = sl->kernelHeight;

                ar(CEREAL_NVP(inputWidth),
                   CEREAL_NVP(inputHeight),
                   CEREAL_NVP(inputChannels),
                   CEREAL_NVP(outputChannels),
                   CEREAL_NVP(kernelWidth),
                   CEREAL_NVP(kernelHeight));
            }

            // just for this type to be considered polymorphic
            virtual void rtti() {}
        };

        virtual std::shared_ptr<typename Layer<Scalar>::LayerSerialProxy>
            getSerial() {
            return std::make_shared<LayerSerialProxy>(this);
        }
    
    private:
		using Matrix = blaze::DynamicMatrix<Scalar, blaze::columnMajor>;
		using Vector = blaze::DynamicVector<Scalar>;
		using ConstAlignedMapMat = const blaze::CustomMatrix<Scalar, blaze::unaligned, blaze::unpadded, blaze::columnMajor>;
		using ConstAlignedMapVec = const blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;
		using AlignedMapVec = blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;

		const internal::ConvDims m_dim; // Various dimensions of convolution

		size_t inputWidth;
		size_t inputHeight;
        size_t inputChannels;
        size_t outputChannels;
		size_t kernelWidth;
		size_t kernelHeight;
		size_t outputWidth;
		size_t outputHeight;

		std::vector<size_t> jDeltas;
		std::vector<size_t> iDeltas;

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
        Conv2dTranspose(const int inputWidth, const int inputHeight,
                      const int inputChannels, const int outputChannels,
                      const int kernelWidth, const int kernelHeight) :
							Layer<Scalar>(inputWidth * inputHeight * inputChannels,
										  (inputWidth + kernelWidth - 1) * (inputHeight + kernelHeight - 1) * outputChannels),
							m_dim(inputChannels, outputChannels, inputHeight, inputWidth, kernelHeight,
									  kernelWidth),
									  inputWidth(inputWidth), inputHeight(inputHeight),
                                      inputChannels(inputChannels), outputChannels(outputChannels),
									  kernelWidth(kernelWidth), kernelHeight(kernelHeight),
									  outputWidth(inputWidth + kernelWidth - 1),
									  outputHeight(inputHeight + kernelHeight - 1)
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

        Matrix getUnrolledKernel()
        {
            size_t fromWidth = inputWidth + kernelWidth - 1;
            size_t fromHeight = inputHeight + kernelHeight - 1;

            size_t toWidth = inputWidth;
            size_t toHeight = inputHeight;

			Matrix unrolledKernel(toWidth * toHeight, fromWidth * fromHeight, 0);

			size_t dr = fromWidth - kernelWidth;

			jDeltas.resize(unrolledKernel.rows());
			iDeltas.resize(m_filter_data.size());

			Vector kernelRow(kernelWidth * kernelHeight + (kernelHeight - 1) * dr);
			kernelRow = 0;

			size_t p = 0;
			for (size_t i = 0; i < m_filter_data.size(); ++i) {
				kernelRow[p++] = m_filter_data[i];
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
            m_z.resize(this->m_out_size, nobs);

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
            Matrix& dLz = m_z;
            Activation::apply_jacobian(m_z, m_a, next_layer_data, dLz);

            /* Derivative for weights */
            Matrix kernelDerivatives(m_filter_data.size(), nobs, 0);
            for (size_t i = 0; i < nobs; ++i) {
	            Matrix W(outputWidth * outputHeight, inputWidth * inputHeight);
	            for (size_t k = 0; k < dLz.rows(); ++k) {
		            blaze::row(W, k) = blaze::trans(blaze::column(prev_layer_data, i) * dLz(k, i));
	            }
	            blaze::transpose(W);
	            for (size_t j = 0; j < m_filter_data.size(); ++j) {
		            for (size_t k = 0; k < jDeltas.size(); ++k) {
		            	kernelDerivatives(j, i) += W(k, jDeltas[k] + iDeltas[j]);
		            }
	            }
            }

            m_df_data = blaze::mean<blaze::rowwise>(kernelDerivatives);


            /* Derivative for bias */
            ConstAlignedMapMat dLz_by_channel(dLz.data(), outputWidth * outputHeight,
														  m_dim.out_channels * nobs);
            Vector dLb = blaze::trans(blaze::sum<blaze::columnwise>(dLz_by_channel));
            // Average over observations
            ConstAlignedMapMat dLb_by_obs(dLb.data(), m_dim.out_channels, nobs);
            m_db = blaze::mean<blaze::rowwise>(dLb_by_obs);


            m_din.resize(this->m_in_size, nobs);

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


#endif /* LAYER_CONV2D_TRANSPOSE_H_ */
