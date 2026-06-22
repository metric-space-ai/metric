#ifndef LAYER_CONVOLUTIONAL_H_
#define LAYER_CONVOLUTIONAL_H_

#include <chrono>
#include <stdexcept>

#include "../Layer.h"
#include "../Utils/Convolution.h"
#include "../Utils/Random.h"

namespace mtrc::solve::parametric::dnn {

///
/// \ingroup Layers
///
/// Conv2dTranspose hidden layer
///
/// Currently only supports the "valid" rule of convolution.
///
template <typename Scalar, typename Activation> class Conv2d : public Layer<Scalar> {
  public:
	using Matrix = mtrc::numeric::DynamicMatrix<Scalar>;

  protected:
	using SparseMatrix = mtrc::numeric::CompressedMatrix<Scalar, mtrc::numeric::columnMajor>;
	using ColumnMatrix = mtrc::numeric::DynamicMatrix<Scalar, mtrc::numeric::columnMajor>;
	using Vector = mtrc::numeric::DynamicVector<Scalar, mtrc::numeric::rowVector>;
	using SparseVector = mtrc::numeric::CompressedVector<Scalar, mtrc::numeric::rowVector>;
	// using ConstAlignedMapVec = const mtrc::numeric::CustomVector<Scalar, mtrc::numeric::aligned,
	// mtrc::numeric::unpadded>; using AlignedMapVec = mtrc::numeric::CustomVector<Scalar, mtrc::numeric::aligned,
	// mtrc::numeric::unpadded>;

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
	mtrc::numeric::CompressedMatrix<size_t, mtrc::numeric::columnMajor> unrolledKernelMap;
	std::vector<std::vector<size_t>> kernelMasks;

	Vector kernelsData; // Filter parameters. Total length is

	// (in_channels x out_channels x filter_rows x filter_cols)
	// See Utils/Convolution.h for its layout

	Vector df_data; // Derivative of filters, same dimension as m_filter_data

	Vector bias; // Bias term for the output channels, out_channels x 1. (One bias term per channel)
	Vector db;	 // Derivative of bias, same dimension as m_bias

	Matrix z;	// Linear term, z = conv(in, w) + b. Each column is an observation
	Matrix a;	// Output of this layer, a = act(z)
	Matrix din; // Derivative of the input of this layer
				// Note that input of this layer is also the output of previous layer

  public:
	Conv2d(const size_t inputSize, const size_t outputSize) : Layer<Scalar>(inputSize, outputSize) {}

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
	Conv2d(const size_t inputWidth, const size_t inputHeight, const size_t inputChannels, const size_t outputChannels,
		   const size_t kernelWidth, const size_t kernelHeight, const size_t stride = 1, bool isZeroPadding = false)
		: Layer<Scalar>(inputWidth * inputHeight * inputChannels, ((inputWidth - kernelWidth) / stride + 1) *
																	  ((inputHeight - kernelHeight) / stride + 1) *
																	  outputChannels),

		  inputWidth(inputWidth), inputHeight(inputHeight), kernelWidth(kernelWidth), kernelHeight(kernelHeight),
		  inputChannels(inputChannels), outputChannels(outputChannels), stride(stride), isZeroPadding(isZeroPadding),
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

	explicit Conv2d(const mtrc::core::Metadata &json)
		: inputWidth(json["inputWidth"].get<int>()), inputHeight(json["inputHeight"].get<int>()),
		  kernelWidth(json["kernelWidth"].get<int>()), kernelHeight(json["kernelHeight"].get<int>()),
		  inputChannels(json["inputChannels"].get<int>()), outputChannels(json["outputChannels"].get<int>()),
		  stride(json["stride"].get<int>()), isZeroPadding(json.value("zeroPadding", false))

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

	mtrc::core::Metadata toJson() override
	{
		auto json = Layer<Scalar>::toJson();
		json["type"] = isTranspose ? "Conv2dTranspose" : "Conv2d";
		json["activation"] = Activation::getType();
		json["inputWidth"] = inputWidth;
		json["inputHeight"] = inputHeight;
		json["inputChannels"] = inputChannels;
		json["outputChannels"] = outputChannels;
		json["kernelWidth"] = kernelWidth;
		json["kernelHeight"] = kernelHeight;
		json["stride"] = stride;
		json["zeroPadding"] = isZeroPadding;
		return json;
	}

	void init(const Scalar &mu, const Scalar &sigma, std::mt19937 &rng) override
	{
		// Random initialization of filter parameters
		internal::set_normal_random(kernelsData.data(), kernelsData.size(), rng, mu, sigma);

		internal::set_normal_random(bias.data(), outputChannels, rng, mu, sigma);

		getUnrolledKernel();
	}

	void init(const std::map<std::string, std::shared_ptr<Initializer<Scalar>>> initializers) override
	{
		initializers.at("normal")->init(kernelsData.size(), kernelsData);
		initializers.at("zero")->init(bias.size(), bias);
	}

	SparseMatrix constructBaseUnrolledKernel(size_t fromWidth, size_t fromHeight, size_t toWidth, size_t toHeight)
	{
		/* Parse padded input */
		size_t paddingWidth = 0;
		size_t paddingHeight = 0;

		if (isZeroPadding) {
			paddingWidth = (kernelWidth - 1) / 2;
			paddingHeight = (kernelHeight - 1) / 2;
		}

		SparseMatrix unrolledKernel(fromWidth * fromHeight, toWidth * toHeight);

		unrolledKernel.reserve(toHeight * toWidth * kernelHeight * kernelWidth);
		size_t column = 0;
		for (int i0 = -paddingHeight; i0 <= (int)(fromHeight + paddingHeight - kernelHeight); i0 += stride) {
			for (int j0 = -paddingWidth; j0 <= (int)(fromWidth + paddingWidth - kernelWidth); j0 += stride) {

				std::vector<size_t> kernelMask;
				size_t c = 0;
				for (int ki = 0; ki < kernelHeight; ++ki) {
					for (int kj = 0; kj < kernelWidth; ++kj) {

						int i = i0 + ki;
						int j = j0 + kj;
						if ((i < 0) || (i >= fromHeight) || (j < 0) || (j >= fromWidth)) {
							kernelMask.push_back(c);
						} else {
							unrolledKernel.append(i * fromWidth + j, column, ki * kernelWidth + kj + 1);
						}

						++c;
					}
				}
				unrolledKernel.finalize(column);
				kernelMasks.push_back(kernelMask);

				++column;
			}
		}

		return unrolledKernel;
	}

	void calculateUnrolledKernelStructure()
	{
		kernelMasks.clear();

		/* Init unrolled kernels */
		unrolledKernels.resize(outputChannels,
							   SparseMatrix(inputChannels * inputWidth * inputHeight, outputWidth * outputHeight));

		SparseMatrix unrolledKernel0;
		if (isTranspose) {
			unrolledKernel0 =
				mtrc::numeric::trans(constructBaseUnrolledKernel(outputWidth, outputHeight, inputWidth, inputHeight));
			unrolledKernelMap = unrolledKernel0;
		} else {
			unrolledKernel0 = constructBaseUnrolledKernel(inputWidth, inputHeight, outputWidth, outputHeight);
		}

		for (auto &unrolledKernel : unrolledKernels) {
			for (size_t inputChannel = 0; inputChannel < inputChannels; ++inputChannel) {
				mtrc::numeric::submatrix(unrolledKernel, inputChannel * inputHeight * inputWidth, 0,
										   inputHeight * inputWidth, unrolledKernel.columns()) = unrolledKernel0;
			}
		}
	}

	void getUnrolledKernel()
	{
		const size_t kernelOneLength = kernelWidth * kernelHeight;
		const size_t kernelOutputChannelLength = inputChannels * kernelOneLength;

		for (size_t outputChannel = 0; outputChannel < outputChannels; ++outputChannel) {
			auto &unrolledKernelInputChannels = unrolledKernels[outputChannel];
			auto kernelDataInputChannels = mtrc::numeric::subvector(
				kernelsData, outputChannel * kernelOutputChannelLength, kernelOutputChannelLength);

			const size_t unrolledKernelRowsNumber = unrolledKernelInputChannels.rows() / inputChannels;
			for (size_t inputChannel = 0; inputChannel < inputChannels; ++inputChannel) {
				auto unrolledKernel =
					mtrc::numeric::submatrix(unrolledKernelInputChannels, inputChannel * unrolledKernelRowsNumber, 0,
											   unrolledKernelRowsNumber, unrolledKernelInputChannels.columns());

				auto kernelData = mtrc::numeric::subvector(kernelDataInputChannels, inputChannel * kernelOneLength,
															 kernelOneLength);

				/* Fill unrolled kernel */
				if (isTranspose) {
					for (size_t i = 0; i < unrolledKernel.columns(); ++i) {
						// Reachable from the public setParameters -> getUnrolledKernel path;
						// surface an internal invariant violation as an exception rather than
						// aborting (and never silently skipping it under NDEBUG).
						if (mtrc::numeric::column(unrolledKernel, i).nonZeros() !=
							mtrc::numeric::column(unrolledKernelMap, i).nonZeros()) {
							throw std::runtime_error(
								"Conv2dTranspose unrolled kernel sparsity does not match the precomputed map");
						}

						for (auto [element, map] = std::tuple(unrolledKernel.begin(i), unrolledKernelMap.begin(i));
							 element != unrolledKernel.end(i); ++element, ++map) {
							element->value() = kernelData[map->value() - 1];
						}
					}
				} else {
					for (size_t i = 0; i < unrolledKernel.columns(); ++i) {
						const auto &mask = kernelMasks[i];
						auto me = mask.begin();
						auto element = unrolledKernel.begin(i);
						for (auto k = 0; k < kernelData.size(); ++k) {
							if (me != mask.end() && (*me == k)) {
								++me;
							} else {
								if (element == unrolledKernel.end(i)) {
									throw std::runtime_error("Conv2d unrolled kernel structure is inconsistent");
								}
								element++->value() = kernelData[k];
							}
						}
					}
				}
			}
		}
	}

	void forward(const Matrix &prev_layer_data) override
	{
		// Each row is an observation
		const size_t nobs = prev_layer_data.rows();
		if (prev_layer_data.columns() != this->inputSize) {
			throw std::invalid_argument("Conv2d input data have incorrect dimension");
		}

		// Linear term, z = conv(in, w) + b
		z.resize(nobs, this->outputSize);

		// cout << prev_layer_data;
		// cout << getUnrolledKernel() << endl;
		// cout << m_filter_data << endl;
		// cout << unrolledKernel << endl;
		const size_t channel_nelem = outputWidth * outputHeight;

		for (size_t channel = 0; channel < outputChannels; ++channel) {
			mtrc::numeric::submatrix(z, 0, channel * channel_nelem, nobs, channel_nelem) =
				prev_layer_data * unrolledKernels[channel];
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

	const Matrix &output() const override { return a; }

	// prev_layer_data: getInputSize x nobs
	// next_layer_data: getOutputSize x nobs
	// https://grzegorzgwardys.wordpress.com/2016/04/22/8/
	void backprop(const Matrix &prev_layer_data, const Matrix &next_layer_data) override
	{
		const size_t nobs = prev_layer_data.rows();
		if (prev_layer_data.columns() != this->inputSize) {
			throw std::invalid_argument("Conv2d backprop input data have incorrect dimension");
		}
		if (next_layer_data.rows() != nobs || next_layer_data.columns() != this->outputSize) {
			throw std::invalid_argument("Conv2d backprop gradient data have incorrect dimension");
		}

		ColumnMatrix dLz = z;
		// Matrix dLz = z;
		Activation::apply_jacobian(z, a, next_layer_data, dLz);

		const size_t outputChannelElementsNumber = outputWidth * outputHeight;
		const size_t kernelOutputChannelLength = inputChannels * kernelWidth * kernelHeight;

		auto t1 = std::chrono::high_resolution_clock::now();

		/* Derivative for kernel */
		ColumnMatrix kernelDerivatives(nobs, kernelsData.size(), 0);
		const size_t inputChannelElementsNumber = inputWidth * inputHeight;
		const size_t kernelOneLength = kernelWidth * kernelHeight;

		/* Parse output channels */
		for (size_t outputChannel = 0; outputChannel < outputChannels; ++outputChannel) {
			auto dLzChannel = mtrc::numeric::submatrix(dLz, 0, outputChannel * outputChannelElementsNumber, nobs,
														 outputChannelElementsNumber);
			auto kernelDerivativesChannel = mtrc::numeric::submatrix(
				kernelDerivatives, 0, outputChannel * kernelOutputChannelLength, nobs, kernelOutputChannelLength);
			for (size_t observation = 0; observation < nobs; ++observation) {
				/* Parse input channels */
				for (size_t inputChannel = 0; inputChannel < inputChannels; ++inputChannel) {
					auto unrolledKernel =
						mtrc::numeric::submatrix(unrolledKernels[outputChannel],
												   inputChannel * inputChannelElementsNumber, 0,
												   inputChannelElementsNumber, outputChannelElementsNumber);

					for (size_t outputElement = 0; outputElement < outputChannelElementsNumber; ++outputElement) {
						const auto &mask = kernelMasks[outputElement];
						auto maskElement = mask.begin();
						auto inputElement = unrolledKernel.begin(outputElement);

						for (size_t kernelElement = 0; kernelElement < kernelOneLength; ++kernelElement) {
							if (maskElement != mask.end() && *maskElement == kernelElement) {
								++maskElement;
								continue;
							}
							if (inputElement == unrolledKernel.end(outputElement)) {
								throw std::runtime_error("Conv2d kernel derivative structure is inconsistent");
							}

							const size_t inputFeature =
								inputChannel * inputChannelElementsNumber + inputElement->index();
							const size_t kernelFeature = inputChannel * kernelOneLength + kernelElement;
							kernelDerivativesChannel(observation, kernelFeature) +=
								prev_layer_data(observation, inputFeature) * dLzChannel(observation, outputElement);
							++inputElement;
						}
					}
				}
			}
		}

		auto t2 = std::chrono::high_resolution_clock::now();
		auto d = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		// std::cout << "time: " << d.count() << " s" << std::endl;

		/* Average over observations */
		df_data = mtrc::numeric::mean<mtrc::numeric::columnwise>(kernelDerivatives);

		/* Derivative for bias */
		for (size_t channel = 0; channel < outputChannels; ++channel) {
			mtrc::numeric::DynamicVector<Scalar> d =
				mtrc::numeric::sum<mtrc::numeric::rowwise>(mtrc::numeric::submatrix(
					dLz, 0, channel * outputChannelElementsNumber, nobs, outputChannelElementsNumber));
			db[channel] = mtrc::numeric::mean(d);
		}

		/* Derivative for input */
		din.resize(nobs, this->inputSize);
		din = Scalar(0);
		for (size_t channel = 0; channel < outputChannels; ++channel) {
			din += mtrc::numeric::submatrix(dLz, 0, channel * outputChannelElementsNumber, nobs,
											  outputChannelElementsNumber) *
				   mtrc::numeric::trans(unrolledKernels[channel]);
		}
	}

	const Matrix &backprop_data() const override { return din; }

	void update(Optimizer<Scalar> &opt) override
	{
		/*ConstAlignedMapVec dw(df_data.data(), df_data.size());
		ConstAlignedMapVec dbConst(db.data(), db.size());
		AlignedMapVec      w(kernelsData.data(), kernelsData.size());
		AlignedMapVec      b(bias.data(), bias.size());*/

		opt.update(df_data, kernelsData);
		opt.update(db, bias);

		getUnrolledKernel();
	}

	std::vector<std::vector<Scalar>> getParameters() const override
	{
		std::vector<std::vector<Scalar>> parameters(2);

		// Size the destination groups before copying. The vectors are
		// default-constructed (empty), so copying into begin() without a prior
		// resize is an out-of-bounds write on the Network::save path. Mirror the
		// FullyConnected::getParameters contract: parameters[0] = kernels,
		// parameters[1] = bias.
		parameters[0].resize(mtrc::numeric::size(kernelsData));
		parameters[1].resize(mtrc::numeric::size(bias));

		// kernels
		std::copy(kernelsData.data(), kernelsData.data() + kernelsData.size(), parameters[0].begin());
		// bias
		std::copy(bias.data(), bias.data() + bias.size(), parameters[1].begin());

		return parameters;
	}

	void setParameters(const std::vector<std::vector<Scalar>> &parameters) override
	{
		/*if (static_cast<int>(param.size()) != mtrc::numeric::size(m_weight) + m_bias.size())
		{
			throw std::invalid_argument("Parameter size does not match");
		}*/

		// Public path (e.g. Network::load on deserialized artifacts): reject shape
		// mismatches with an explicit exception instead of asserting/aborting, which
		// would otherwise corrupt memory under NDEBUG where asserts are stripped.
		if (parameters.size() != 2) {
			throw std::invalid_argument("Conv2d::setParameters expects a kernel group and a bias group");
		}
		if (parameters[0].size() != mtrc::numeric::size(kernelsData)) {
			throw std::invalid_argument("Conv2d::setParameters kernel size does not match layer shape");
		}
		if (parameters[1].size() != mtrc::numeric::size(bias)) {
			throw std::invalid_argument("Conv2d::setParameters bias size does not match layer shape");
		}

		std::copy(parameters[0].begin(), parameters[0].begin() + mtrc::numeric::size(kernelsData),
				  kernelsData.data());
		std::copy(parameters[1].begin(), parameters[1].begin() + mtrc::numeric::size(bias), bias.data());

		getUnrolledKernel();
	}

	std::vector<Scalar> get_derivatives() const override
	{
		std::vector<Scalar> res(df_data.size() + db.size());
		// Copy the data of filters and bias to a long vector
		std::copy(df_data.data(), df_data.data() + df_data.size(), res.begin());
		std::copy(db.data(), db.data() + db.size(), res.begin() + df_data.size());
		return res;
	}

	std::vector<size_t> getOutputShape() const override { return {outputWidth, outputHeight}; }
};

} // namespace mtrc::solve::parametric::dnn

#endif /* LAYER_CONVOLUTIONAL_H_ */
