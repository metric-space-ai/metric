#ifndef LAYER_H_
#define LAYER_H_

#include <vector>
#include <random>

#include "../../../3rdparty/blaze/Math.h"
#include "../../../3rdparty/json/json.hpp"

#include "Optimizer.h"

namespace metric::dnn
{


///
/// \defgroup Layers Hidden Layers
///

///
/// \ingroup Layers
///
/// The interface of hidden layers in a neural network. It defines some common
/// operations of hidden layers such as initialization, forward and backward
/// propogation, and also functions to get/set parameters of the layer.
///
template<typename Scalar>
class Layer
{
    public:
		using Matrix = blaze::DynamicMatrix<Scalar>;

		const int inputSize;  // Size of input units
		const int outputSize; // Size of output units

		///
        /// Constructor
        ///
        /// \param inputSize  Number of input units of this hidden Layer. It must be
        ///                 equal to the number of output units of the previous layer.
        /// \param outputSize Number of output units of this hidden layer. It must be
        ///                 equal to the number of input units of the next layer.
        ///
        Layer(const int inputSize, const int outputSize) :
		        inputSize(inputSize), outputSize(outputSize)
        {}

        ///
        /// Virtual destructor
        ///
        virtual ~Layer() {}

        Layer(const nlohmann::json& json) : inputSize(json["inputSize"].get<int>()),
                                                outputSize(json["outputSize"].get<int>())
        {}

	virtual nlohmann::json toJson()
		{
			nlohmann::json json = {
					{"inputSize", inputSize},
					{"outputSize", outputSize}
			};
			return json;
		}
	///
	/// Get the number of input units of this hidden layer.
        ///
        int getInputSize() const
        {
            return inputSize;
        }
        ///
        /// Get the number of output units of this hidden layer.
        ///
        int getOutputSize() const
        {
            return outputSize;
        }

        ///
        /// Initialize layer parameters using \f$N(\mu, \sigma^2)\f$ distribution
        ///
        /// \param mu    Mean of the normal distribution.
        /// \param sigma Standard deviation of the normal distribution.
        /// \param rng   The random number generator of type std::mt19937.
        virtual void init(const Scalar& mu, const Scalar& sigma, std::mt19937& rng) = 0;

        //virtual void initConstant(const Scalar weightsValue, const Scalar biasesValue) = 0;

        ///
        /// Compute the output of this layer
        ///
        /// The purpose of this function is to let the hidden layer compute information
        /// that will be passed to the next layer as the input. The concrete behavior
        /// of this function is subject to the implementation, with the only
        /// requirement that after calling this function, the Layer::output() member
        /// function will return a reference to the output values.
        ///
        /// \param prev_layer_data The output of previous layer, which is also the
        ///                        input of this layer. `prev_layer_data` should have
        ///                        `getInputSize` rows as in the constructor, and each
        ///                        column of `prev_layer_data` is an observation.
        ///
        virtual void forward(const Matrix& prev_layer_data) = 0;

        ///
        /// Obtain the output values of this layer
        ///
        /// This function is assumed to be called after Layer::forward() in each iteration.
        /// The output are the values of output hidden units after applying activation function.
        /// The main usage of this function is to provide the `prev_layer_data` parameter
        /// in Layer::forward() of the next layer.
        ///
        /// \return A reference to the matrix that contains the output values. The
        ///         matrix should have `getOutputSize` rows as in the constructor,
        ///         and have number of columns equal to that of `prev_layer_data` in the
        ///         Layer::forward() function. Each column represents an observation.
        ///
        virtual const Matrix& output() const = 0;

        ///
        /// Compute the gradients of parameters and input units using back-propagation
        ///
        /// The purpose of this function is to compute the gradient of input units,
        /// which can be retrieved by Layer::backprop_data(), and the gradient of
        /// layer parameters, which could later be used by the Layer::update() function.
        ///
        /// \param prev_layer_data The output of previous layer, which is also the
        ///                        input of this layer. `prev_layer_data` should have
        ///                        `getInputSize` rows as in the constructor, and each
        ///                        column of `prev_layer_data` is an observation.
        /// \param next_layer_data The gradients of the input units of the next layer,
        ///                        which is also the gradients of the output units of
        ///                        this layer. `next_layer_data` should have
        ///                        `getOutputSize` rows as in the constructor, and the same
        ///                        number of columns as `prev_layer_data`.
        ///
        virtual void backprop(const Matrix& prev_layer_data,
                              const Matrix& next_layer_data) = 0;

        ///
        /// Obtain the gradient of input units of this layer
        ///
        /// This function provides the `next_layer_data` parameter in Layer::backprop()
        /// of the previous layer, since the derivative of the input of this layer is also the derivative
        /// of the output of previous layer.
        ///
        virtual const Matrix& backprop_data() const = 0;

        ///
        /// Update parameters after back-propagation
        ///
        /// \param opt The optimization algorithm to be used. See the Optimizer class.
        ///
        virtual void update(Optimizer<Scalar>& opt) = 0;

        ///
        /// Get serialized values of parameters
        ///
        virtual std::vector<std::vector<Scalar>> getParameters() const = 0;
        ///
        /// Set the values of layer parameters from serialized data
        ///
        virtual void setParameters(const std::vector<std::vector<Scalar>>& param) {};

        ///
        /// Get serialized values of the gradient of parameters
        ///
        virtual std::vector<Scalar> get_derivatives() const = 0;
};


} // namespace metric::dnn


#endif /* LAYER_H_ */
