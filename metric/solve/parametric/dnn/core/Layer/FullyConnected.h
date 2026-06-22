#ifndef LAYER_FULLYCONNECTED_H_
#define LAYER_FULLYCONNECTED_H_

#include <iostream>
#include <random>
#include <stdexcept>

#include <metric/numeric/Math.h>

#include "../Layer.h"
#include "../Utils/Random.h"

namespace mtrc::solve::parametric::dnn {

///
/// \ingroup Layers
///
/// Fully connected hidden layer
///
template <typename Scalar, typename Activation> class FullyConnected : public Layer<Scalar> {
  private:
	using Matrix = mtrc::numeric::DynamicMatrix<Scalar>;
	using ColumnMatrix = mtrc::numeric::DynamicMatrix<Scalar, mtrc::numeric::columnMajor>;
	using Vector = mtrc::numeric::DynamicVector<Scalar, mtrc::numeric::rowVector>;
	using ConstAlignedMapVec =
		const mtrc::numeric::CustomVector<Scalar, mtrc::numeric::aligned, mtrc::numeric::unpadded>;
	using AlignedMapVec = mtrc::numeric::CustomVector<Scalar, mtrc::numeric::aligned, mtrc::numeric::unpadded>;

	ColumnMatrix m_weight; // Weight parameters, W(getInputSize x getOutputSize)
	Vector m_bias;		   // Bias parameters, b(getOutputSize x 1)
	ColumnMatrix m_dw;	   // Derivative of weights
	Vector m_db;		   // Derivative of bias
	Matrix m_z;			   // Linear term, z = W' * in + b
	Matrix m_a;			   // Output of this layer, a = act(z)
	Matrix m_din;		   // Derivative of the input of this layer.
						   // Note that input of this layer is also the output of previous layer

  public:
	///
	/// Constructor
	///
	/// \param in_size  Number of input units.
	/// \param out_size Number of output units.
	///
	FullyConnected(const size_t in_size, const size_t out_size) : Layer<Scalar>(in_size, out_size) {}

	FullyConnected(const mtrc::core::Metadata &json)
	{
		this->inputSize = json["inputSize"].get<int>();
		this->outputSize = json["outputSize"].get<int>();
	}

	mtrc::core::Metadata toJson()
	{
		auto json = Layer<Scalar>::toJson();
		json["type"] = "FullyConnected";
		json["activation"] = Activation::getType();

		return json;
	}

	void init(const Scalar &mu, const Scalar &sigma, std::mt19937 &rng)
	{
		// this->initConstant(0.1, 0);
		// return;
		m_weight.resize(this->inputSize, this->outputSize);
		m_bias.resize(this->outputSize);
		m_dw.resize(this->inputSize, this->outputSize);
		m_db.resize(this->outputSize);

		// Set random coefficients
		internal::set_normal_random(m_weight.data(), mtrc::numeric::size(m_weight), rng, mu, sigma);
		internal::set_normal_random(m_bias.data(), m_bias.size(), rng, mu, sigma);

		// m_weight = 1;
		// m_bias = 0.01;
	}

	void init(const std::map<std::string, std::shared_ptr<Initializer<Scalar>>> initializers)
	{
		initializers.at("normal")->init(this->inputSize, this->outputSize, m_weight);
		initializers.at("zero")->init(this->outputSize, m_bias);

		m_dw.resize(this->inputSize, this->outputSize);
		m_db.resize(this->outputSize);
	}

	void initConstant(const Scalar weightsValue, const Scalar biasesValue)
	{
		m_weight.resize(this->inputSize, this->outputSize);
		m_bias.resize(this->outputSize);
		m_dw.resize(this->inputSize, this->outputSize);
		m_db.resize(this->outputSize);

		m_weight = weightsValue;
		m_bias = biasesValue;
	}

	// prev_layer_data: getInputSize x nobs
	void forward(const Matrix &prev_layer_data)
	{
		const size_t nobs = prev_layer_data.rows();

		// Linear term z = W' * in + b
		m_z.resize(nobs, this->outputSize);

		m_z = prev_layer_data * m_weight;
		for (size_t i = 0UL; i < m_z.rows(); i++) {
			mtrc::numeric::row(m_z, i) += m_bias;
		}
		// Apply activation function
		m_a.resize(nobs, this->outputSize);
		// std::cout << mtrc::numeric::submatrix<0, 0, 5, 20>(prev_layer_data) << std::endl;
		// std::cout << mtrc::numeric::submatrix<0, 0, 5, 20>(m_z) << std::endl;
		Activation::activate(m_z, m_a);
	}

	const Matrix &output() const { return m_a; }

	// prev_layer_data: getInputSize x nobs
	// next_layer_data: getOutputSize x nobs
	void backprop(const Matrix &prev_layer_data, const Matrix &next_layer_data)
	{
		const size_t nobs = prev_layer_data.rows();
		// After forward stage, m_z contains z = W' * in + b
		// Now we need to calculate d(L) / d(z) = [d(a) / d(z)] * [d(L) / d(a)]
		// d(L) / d(a) is computed in the next layer, contained in next_layer_data
		// The Jacobian matrix J = d(a) / d(z) is determined by the activation function
		ColumnMatrix dLz = m_z;

		Activation::apply_jacobian(m_z, m_a, next_layer_data, dLz);

		// Now dLz contains d(L) / d(z)
		// Derivative for weights, d(L) / d(W) = [d(L) / d(z)] * in'
		m_dw = mtrc::numeric::trans(prev_layer_data) * dLz / nobs;

		// Derivative for bias, d(L) / d(b) = d(L) / d(z)
		m_db = mtrc::numeric::mean<mtrc::numeric::columnwise>(dLz);

		// Compute d(L) / d_in = W * [d(L) / d(z)]
		m_din.resize(nobs, this->inputSize);
		m_din = dLz * mtrc::numeric::trans(m_weight);

		//	        std::cout << "m_dw" << std::endl;
		//	        std::cout << m_dw << std::endl;
		//	        std::cout << "m_b" << std::endl;
		//	        std::cout << m_db << std::endl;
	}

	const Matrix &backprop_data() const { return m_din; }

	void update(Optimizer<Scalar> &opt)
	{
		/*
					ConstAlignedMapVec dw(m_dw.data(), mtrc::numeric::size(m_dw));
					ConstAlignedMapVec db(m_db.data(), m_db.size());
					AlignedMapVec      w(m_weight.data(), mtrc::numeric::size(m_weight));
					AlignedMapVec      b(m_bias.data(), m_bias.size());
		*/
		opt.update(m_dw, m_weight);
		opt.update(m_db, m_bias);
	}

	std::vector<std::vector<Scalar>> getParameters() const
	{
		std::vector<std::vector<Scalar>> parameters(2);

		parameters[0].resize(mtrc::numeric::size(m_weight));
		parameters[1].resize(m_bias.size());

		size_t offset = 0;
		for (size_t column = 0; column < m_weight.columns(); ++column) {
			for (size_t row = 0; row < m_weight.rows(); ++row) {
				parameters[0][offset++] = m_weight(row, column);
			}
		}
		std::copy(m_bias.data(), m_bias.data() + m_bias.size(), parameters[1].begin());

		return parameters;
	}

	void setParameters(const std::vector<std::vector<Scalar>> &parameters)
	{
		// Public path (e.g. Network::load on deserialized artifacts): reject shape
		// mismatches with an explicit exception instead of asserting/aborting, which
		// would otherwise corrupt memory under NDEBUG where asserts are stripped.
		if (parameters.size() != 2) {
			throw std::invalid_argument("FullyConnected::setParameters expects a weight group and a bias group");
		}
		if (parameters[0].size() != mtrc::numeric::size(m_weight)) {
			throw std::invalid_argument("FullyConnected::setParameters weight size does not match layer shape");
		}
		if (parameters[1].size() != mtrc::numeric::size(m_bias)) {
			throw std::invalid_argument("FullyConnected::setParameters bias size does not match layer shape");
		}

		size_t offset = 0;
		for (size_t column = 0; column < m_weight.columns(); ++column) {
			for (size_t row = 0; row < m_weight.rows(); ++row) {
				m_weight(row, column) = parameters[0][offset++];
			}
		}
		std::copy(parameters[1].begin(), parameters[1].begin() + mtrc::numeric::size(m_bias), m_bias.data());
	}

	std::vector<Scalar> get_derivatives() const
	{
		std::vector<Scalar> res(mtrc::numeric::size(m_dw) + m_db.size());
		// Copy the data of weights and bias to a long vector
		std::copy(m_dw.data(), m_dw.data() + mtrc::numeric::size(m_dw), res.begin());
		std::copy(m_db.data(), m_db.data() + m_db.size(), res.begin() + mtrc::numeric::size(m_dw));
		return res;
	}

	std::vector<size_t> getOutputShape() const { return {(size_t)this->outputSize}; }
};

} // namespace mtrc::solve::parametric::dnn

#endif /* LAYER_FULLYCONNECTED_H_ */
