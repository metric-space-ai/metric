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
    public:
        using Matrix = blaze::DynamicMatrix<Scalar>;

    protected:
        using SparseMatrix = blaze::CompressedMatrix<Scalar, blaze::columnMajor>;
        using ColumnMatrix = blaze::DynamicMatrix<Scalar, blaze::columnMajor>;
        using Vector = blaze::DynamicVector<Scalar, blaze::rowVector>;
        using SparseVector = blaze::CompressedVector<Scalar, blaze::rowVector>;
        //using ConstAlignedMapVec = const blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;
        //using AlignedMapVec = blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;


        size_t inputWidth;
        size_t inputHeight;
        size_t kernelWidth;
        size_t kernelHeight;
        size_t outputWidth;
        size_t outputHeight;
        size_t inputChannels;
        size_t outputChannels;
        size_t stride;
		bool isZeroPadding;

        bool isTranspose;

        std::vector<SparseMatrix> unrolledKernels;
        std::vector<size_t> jDeltas;
        std::vector<size_t> iDeltas;
		std::vector<std::vector<size_t>> kernelMasks;

	Vector kernelsData;           // Filter parameters. Total length is
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
        Conv2d(const size_t inputSize, const size_t outputSize) : Layer<Scalar>(inputSize, outputSize)
        {

        }

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
        Conv2d(const size_t inputWidth, const size_t inputHeight,
               const size_t inputChannels, const size_t outputChannels,
               const size_t kernelWidth, const size_t kernelHeight,
               const size_t stride = 1, bool isZeroPadding = false) :
                                            Layer<Scalar>(inputWidth * inputHeight * inputChannels,
                                              ((inputWidth - kernelWidth) / stride + 1) *
                                                ((inputHeight - kernelHeight) / stride + 1) * outputChannels),

                                              inputWidth(inputWidth), inputHeight(inputHeight),
                                              kernelWidth(kernelWidth), kernelHeight(kernelHeight),
                                              inputChannels(inputChannels), outputChannels(outputChannels),
                                              stride(stride), isZeroPadding(isZeroPadding),
                                              outputWidth(isZeroPadding ? inputWidth : (inputWidth - kernelWidth) / stride + 1),
                                              outputHeight(isZeroPadding ? inputHeight : (inputHeight - kernelHeight) / stride + 1)
        {
            this->inputSize = inputChannels * inputWidth * inputHeight;
            this->outputSize = outputChannels * outputWidth * outputHeight;

            // Set data dimension
            const size_t kernelDataSize = inputChannels * outputChannels * kernelWidth * kernelHeight;

            kernelsData.resize(kernelDataSize);
            df_data.resize(kernelDataSize);

            // Bias term
            bias.resize(outputChannels);
            db.resize(outputChannels);

            isTranspose = false;

            calculateUnrolledKernelStructure();
            getUnrolledKernel();
        }

        Conv2d(const nlohmann::json& json) : inputWidth(json["inputWidth"].get<int>()),
                                             inputHeight(json["inputHeight"].get<int>()),
                                             kernelWidth(json["kernelWidth"].get<int>()),
                                             kernelHeight(json["kernelHeight"].get<int>()),
                                             inputChannels(json["inputChannels"].get<int>()),
                                             outputChannels(json["outputChannels"].get<int>()),
                                             stride(json["stride"].get<int>()),
                                             isZeroPadding(json["zeroPadding"].get<bool>())
        {
            outputWidth = isZeroPadding ? inputWidth : (inputWidth - kernelWidth) / stride + 1;
            outputHeight = isZeroPadding ? inputHeight : (inputHeight - kernelHeight) / stride + 1;

            this->inputSize = inputChannels * inputWidth * inputHeight;
            this->outputSize = outputChannels * outputWidth * outputHeight;

            // Set data dimension
            const size_t kernelDataSize = inputChannels * outputChannels * kernelWidth * kernelHeight;

            kernelsData.resize(kernelDataSize);
            df_data.resize(kernelDataSize);

            // Bias term
            bias.resize(outputChannels);
            db.resize(outputChannels);

            isTranspose = false;
            calculateUnrolledKernelStructure();
            getUnrolledKernel();
        }

        void init(const Scalar& mu, const Scalar& sigma, std::mt19937& rng)
        {
            // Random initialization of filter parameters
            internal::set_normal_random(kernelsData.data(), kernelsData.size(), rng, mu, sigma);

            internal::set_normal_random(bias.data(), outputChannels, rng, mu, sigma);

            getUnrolledKernel();
        }

        void init(const std::map<std::string, std::shared_ptr<Initializer<Scalar>>> initializers)
        {
            initializers.at("normal")->init(kernelsData.size(), kernelsData);
            initializers.at("zero")->init(bias.size(), bias);
        }

        void calculateUnrolledKernelStructure()
        {
            /* Init unrolled kernels */
            unrolledKernels.resize(outputChannels, SparseMatrix(inputChannels * inputWidth * inputHeight,
                                                                outputWidth * outputHeight));

            //for (auto j = 0; j < unrolledKernel.columns(); ++j) {
            //	unrolledKernel.reserve(j, kernelWidth * kernelHeight);
                    //unrolledKernel.finalize(j);
            //}


            /* Parse padded input */
            size_t paddingWidth = (kernelWidth - 1) / 2;
	        size_t paddingHeight = (kernelHeight - 1) / 2;

	        auto unrolledKernel0 = SparseMatrix(inputWidth * inputHeight, outputWidth * outputHeight);

	        unrolledKernel0.reserve(outputHeight * outputWidth * kernelHeight * kernelWidth);
	        size_t column = 0;
	        for (int i0 = - paddingHeight; i0 <= (int)(inputHeight + paddingHeight - kernelHeight); i0 += stride) {
		        for (int j0 = - paddingWidth; j0 <= (int)(inputWidth + paddingWidth - kernelWidth); j0 += stride) {

		        	std::vector<size_t> kernelMask;
		        	size_t c = 0;
		            for (int ki = 0; ki < kernelHeight; ++ki) {
		            	for (int kj = 0; kj < kernelWidth; ++kj) {

		            		int i = i0 + ki;
		            		int j = j0 + kj;
		            		if ((i < 0) or (i >= inputHeight) or
						            (j < 0) or (j >= inputWidth)) {
		            			kernelMask.push_back(c);
		            		} else {
								unrolledKernel0.append(i * inputWidth + j, column, 1);
		            		}

		            		++c;
		            	}
		            }
		            unrolledKernel0.finalize(column);
		            kernelMasks.push_back(kernelMask);

		            ++column;
		        }
	        }
	        std::cout << unrolledKernel0 << std::endl;

	        for (auto& unrolledKernel: unrolledKernels) {
	        	for (size_t inputChannel = 0; inputChannel < inputChannels; ++inputChannel) {
	        		blaze::submatrix(unrolledKernel, inputChannel * inputHeight * inputWidth, 0,
			                         inputHeight * inputWidth, unrolledKernel.columns()) = unrolledKernel0;
	        	}
	        }

          /*  size_t p = 0;
            for (size_t i = 0; i < iDeltas.size(); ++i) {
                iDeltas[i] = p++;

                if ((i + 1) % kernelWidth == 0) {
                        p += dr;
                }
            }


            *//* Prepare unrolledKernel and jDeltas *//*
            if (!isTranspose) {
                jDeltas.resize(outputWidth * outputHeight);
            } else {
                jDeltas.resize(inputWidth * inputHeight);
            }

            const size_t fromWidth = isTranspose ? outputWidth : inputWidth;
            size_t j00 = 0;
            size_t j0 = 0;
            for (size_t i = 0; i < jDeltas.size(); ++i) {
                jDeltas[i] = (j00 * fromWidth) + j0;

                j0 += stride;
                if (j0 + kernelWidth > fromWidth) {
                    j0 = 0;
                    j00 += stride;
                }
            }



	        *//* Fill unrolled kernels *//*
	        for (auto& unrolledKernel: unrolledKernels) {
                if (isTranspose) {
                    unrolledKernel.reserve(unrolledKernel.rows() * iDeltas.size());
                    for (size_t i = 0; i < unrolledKernel.rows(); ++i) {
                        //assert(unrolledKernel.rows() == jDeltas.size());
                        //assert(jDeltas[i] + kernelRow.columns() <= unrolledKernel.columns());
                        for (size_t id = 0; id < iDeltas.size(); ++id) {
                            unrolledKernel.append(i, jDeltas[i] + iDeltas[id], 1);
                        }
                        unrolledKernel.finalize(i);
                    }
                } else {
                    unrolledKernel.reserve(unrolledKernel.columns() * iDeltas.size());
                    for (size_t i = 0; i < unrolledKernel.columns(); ++i) {
                        for (size_t id = 0; id < iDeltas.size(); ++id) {
                            unrolledKernel.append(jDeltas[i] + iDeltas[id], i, 1);
                        }
                        unrolledKernel.finalize(i);
                    }
                }
            }*/
        }

        void getUnrolledKernel()
        {
            const size_t kernelOneLength = kernelWidth * kernelHeight;
            const size_t kernelOutputChannelLength = inputChannels * kernelOneLength;

            for (size_t outputChannel = 0; outputChannel < outputChannels; ++outputChannel) {
                auto &unrolledKernelInputChannels = unrolledKernels[outputChannel];
                auto kernelDataInputChannels = blaze::subvector(kernelsData, outputChannel * kernelOutputChannelLength, kernelOutputChannelLength);

                const size_t unrolledKernelRowsNumber = unrolledKernelInputChannels.rows() / inputChannels;
                for (size_t inputChannel = 0; inputChannel < inputChannels; ++inputChannel) {
                    auto unrolledKernel = blaze::submatrix(unrolledKernelInputChannels,
                                                            inputChannel * unrolledKernelRowsNumber, 0,
                                                                unrolledKernelRowsNumber, unrolledKernelInputChannels.columns());

                    auto kernelData = blaze::subvector(kernelDataInputChannels, inputChannel * kernelOneLength, kernelOneLength);


                    /* Construct kernel row (convolutional for one output pixel) */
                    //for (size_t i = 0; i < iDeltas.size(); ++i) {
                    //    kernelRow(0, iDeltas[i]) = kernelData[i];
                    //}

                    /* Fill unrolled kernel */
                    if (isTranspose) {
                        //std::cout << unrolledKernel << std::endl;
                        for (size_t i = 0; i < unrolledKernel.rows(); ++i) {
                            //assert(unrolledKernel.rows() == jDeltas.size());
                            //assert(jDeltas[i] + kernelRow.columns() <= unrolledKernel.columns());
                            for (auto [element, kd] = std::tuple(unrolledKernel.begin(i), kernelData.begin()); element != unrolledKernel.end(i); ++element, ++kd) {
                                element->value() = *kd;
                            }
                        }
                    } else {
                        for (size_t i = 0; i < unrolledKernel.columns(); ++i) {
							auto me = kernelMasks[i].begin();
							auto element = unrolledKernel.begin(i);
							for (auto k = 0; k < kernelData.size(); ++k) {
								if (!kernelMasks[i].empty() and (*me == k)) {
									++me;
								} else {
		                            element++->value() = kernelData[k];
	                            }
                            }
                        }
                    }
                }
            }
        }


        void forward(const Matrix& prev_layer_data)
        {
            // Each row is an observation
            const size_t nobs = prev_layer_data.rows();

            // Linear term, z = conv(in, w) + b
            z.resize(nobs, this->outputSize);


            //cout << prev_layer_data;
            //cout << getUnrolledKernel() << endl;
            //cout << m_filter_data << endl;
            //cout << unrolledKernel << endl;
            const size_t channel_nelem = outputWidth * outputHeight;

            for (size_t channel = 0; channel < outputChannels; ++channel) {
                blaze::submatrix(z, 0, channel * channel_nelem, nobs, channel_nelem) = prev_layer_data * unrolledKernels[channel];
            }


	    // Add bias terms
            // Each row of z contains m_dim.out_channels channels, and each channel has
            // m_dim.conv_rows * m_dim.conv_cols elements
            size_t channel_start_row = 0;
            for (size_t i = 0; i < outputChannels; i++, channel_start_row += channel_nelem) {
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

            ColumnMatrix dLz = z;
	        //Matrix dLz = z;
	        Activation::apply_jacobian(z, a, next_layer_data, dLz);

	        const size_t outputChannelElementsNumber = outputWidth * outputHeight;
	        const size_t kernelOutputChannelLength = inputChannels * kernelWidth * kernelHeight;


	        auto t1 = std::chrono::high_resolution_clock::now();

            /* Derivative for kernel */
            ColumnMatrix kernelDerivatives(nobs, kernelsData.size(), 0);

            /* Parse output channels */
	        for (size_t outputChannel = 0; outputChannel < outputChannels; ++outputChannel) {
	        	auto dLzChannel = blaze::submatrix(dLz, 0, outputChannel * outputChannelElementsNumber, nobs, outputChannelElementsNumber);
		        auto kernelDerivativesChannel = blaze::submatrix(kernelDerivatives, 0, outputChannel * kernelOutputChannelLength,
		                                                         nobs, kernelOutputChannelLength);

		        ColumnMatrix W(inputChannels * inputWidth * inputHeight, outputChannelElementsNumber);
				for (size_t observation = 0; observation < nobs; ++observation) {
					auto previousObservation = blaze::trans(blaze::row(prev_layer_data, observation));
					for (size_t k = 0; k < dLzChannel.columns(); ++k) {
						blaze::column(W, k) = previousObservation * dLzChannel(observation, k);
					}

					/* Parse input channels */
					for (size_t inputChannel = 0; inputChannel < inputChannels; ++inputChannel) {
						auto kdic = blaze::submatrix(kernelDerivativesChannel, 0, inputChannel * kernelWidth * kernelHeight, nobs, kernelWidth * kernelHeight);
						auto wic = blaze::submatrix(W, inputChannel * inputWidth * inputHeight, 0, inputWidth * inputHeight, W.columns());
						for (size_t k = 0; k < jDeltas.size(); ++k) {
							auto wice = wic.begin(k);
							auto me = kernelMasks[k].begin();
							for (size_t j = 0; j < kdic.columns(); ++j) {
								if (*me == j) {
									++me;
								} else {
									kdic(observation, j) += *wice;
								}
							}
						}
					}
				}
	        }

	        auto t2 = std::chrono::high_resolution_clock::now();
	        auto d = std::chrono::duration_cast < std::chrono::duration < double >> (t2 - t1);
	        //std::cout << "time: " << d.count() << " s" << std::endl;

	        /* Average over observations */
	        df_data = blaze::mean<blaze::columnwise>(kernelDerivatives);


            /* Derivative for bias */
            for (size_t channel = 0; channel < outputChannels; ++channel) {
            	blaze::DynamicVector<Scalar> d = blaze::sum<blaze::rowwise>(blaze::submatrix(dLz, 0, channel * outputChannelElementsNumber, nobs, outputChannelElementsNumber));
            	db[channel] = blaze::mean(d);
            }


	        /* Derivative for input */
	        din.resize(nobs, this->inputSize);
	        for (size_t channel = 0; channel < outputChannels; ++channel) {
		        din += blaze::submatrix(dLz, 0, channel * outputChannelElementsNumber, nobs, outputChannelElementsNumber) * blaze::trans(unrolledKernels[0]);
	        }
        }

        const Matrix& backprop_data() const
        {
            return din;
        }

        void update(Optimizer<Scalar>& opt)
        {
            /*ConstAlignedMapVec dw(df_data.data(), df_data.size());
            ConstAlignedMapVec dbConst(db.data(), db.size());
            AlignedMapVec      w(kernelsData.data(), kernelsData.size());
            AlignedMapVec      b(bias.data(), bias.size());*/

            opt.update(df_data, kernelsData);
            opt.update(db, bias);

            getUnrolledKernel();
        }

		std::vector<std::vector<Scalar>> getParameters() const
		{
			std::vector<std::vector<Scalar>> parameters(2);

            // kernels
            std::copy(kernelsData.data(), kernelsData.data() + kernelsData.size(),
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

			//std::cout << parameters[0].size() << " " << kernelsData.size() << std::endl;
			assert(parameters[0].size() == kernelsData.size());
			assert(parameters[1].size() == bias.size());

			std::copy(parameters[0].begin(), parameters[0].begin() + blaze::size(kernelsData), kernelsData.data());
			std::copy(parameters[1].begin(), parameters[1].begin() + blaze::size(bias), bias.data());

			getUnrolledKernel();
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

		std::vector<size_t> getOutputShape() const
		{
        	return {outputWidth, outputHeight};
		}
};


} // namespace dnn
} // namespace metric


#endif /* LAYER_CONVOLUTIONAL_H_ */
