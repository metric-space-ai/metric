#ifndef LAYER_MAXPOOLING_H_
#define LAYER_MAXPOOLING_H_

#include <stdexcept>

#include "../Layer.h"
#include "../Utils/FindMax.h"

namespace metric
{
namespace dnn
{


///
/// \ingroup Layers
///
/// Max-pooling hidden layer
///
/// Currently only supports the "valid" rule of pooling.
///
template <typename Scalar, typename Activation>
class MaxPooling: public Layer<Scalar>
{
    private:
		using Matrix = blaze::DynamicMatrix<Scalar>;
		using IntegerMatrix = blaze::DynamicMatrix<int>;

		const int inputHeight;
        const int inputWidth;
        const int inputChannels;
        const int poolingHeight;
        const int poolingWidth;

        const int outputHeight;
        const int outputWidth;

        IntegerMatrix m_loc;             // Record the locations of maximums
        Matrix m_z;                  // Max pooling results
        Matrix m_a;                  // Output of this layer, a = act(z)
        Matrix m_din;                // Derivative of the input of this layer.
        // Note that input of this layer is also the output of previous layer

    public:
        // Currently we only implement the "valid" rule
        // https://stackoverflow.com/q/37674306
        ///
        /// Constructor
        ///
        /// \param inputWidth       Width of the input image in each channel.
        /// \param inputHeight      Height of the input image in each channel.
        /// \param inputChannels    Number of input channels.
        /// \param poolingWidth  Width of the pooling window.
        /// \param poolingHeight Height of the pooling window.
        ///
        MaxPooling(const int inputWidth, const int inputHeight, const int inputChannels,
                   const int poolingWidth, const int poolingHeight) :
		        Layer<Scalar>(inputWidth * inputHeight * inputChannels,
		                      (inputWidth / poolingWidth) * (inputHeight / poolingHeight) * inputChannels),
		        inputHeight(inputHeight), inputWidth(inputWidth), inputChannels(inputChannels),
		        poolingHeight(poolingHeight), poolingWidth(poolingWidth),
		        outputHeight(inputHeight / poolingHeight),
		        outputWidth(inputWidth / poolingWidth)
        {}

        //MaxPooling(const nlohmann::json& json) : Layer<Scalar>(json)
        //{}

        void init(const Scalar& mu, const Scalar& sigma, std::mt19937& rng) {}

        void forward(const Matrix& prev_layer_data)
        {
            // Each row is an observation
            const int nobs = prev_layer_data.rows();
            m_loc.resize(nobs, this->outputSize);
            m_z.resize(nobs, this->outputSize);

            // Use m_loc to store the address of each pooling block relative to the beginning of the data
            int* loc_data = m_loc.data();
            const int channel_end = blaze::size(prev_layer_data);
            const int channel_stride = inputHeight * inputWidth;
            const int col_end_gap = inputHeight * poolingWidth * outputWidth;
            const int col_stride = inputHeight * poolingWidth;
            const int row_end_gap = outputHeight * poolingHeight;

	        for (int channel_start = 0; channel_start < channel_end; channel_start += channel_stride) {
		        const int col_end = channel_start + col_end_gap;

		        for (int col_start = channel_start; col_start < col_end; col_start += col_stride) {
			        const int row_end = col_start + row_end_gap;

			        for (int row_start = col_start; row_start < row_end; row_start += poolingHeight, loc_data++) {
				        *loc_data = row_start;
			        }
		        }
	        }

            // Find the location of the max value in each block
            loc_data = m_loc.data();
            const int* const loc_end = loc_data + blaze::size(m_loc);
            Scalar* z_data = m_z.data();
            const Scalar* src = prev_layer_data.data();

	        for (; loc_data < loc_end; loc_data++, z_data++) {
		        const int offset = *loc_data;
		        *z_data = internal::find_block_max(src + offset, poolingHeight, poolingWidth,
		                                           inputHeight, *loc_data);
		        *loc_data += offset;
	        }

            // Apply activation function
            m_a.resize(this->outputSize, nobs);
            Activation::activate(m_z, m_a);
        }

        const Matrix& output() const
        {
            return m_a;
        }

        // prev_layer_data: getInputSize x nobs
        // next_layer_data: getOutputSize x nobs
        void backprop(const Matrix& prev_layer_data, const Matrix& next_layer_data)
        {
            const int nobs = prev_layer_data.rows();
            // After forward stage, m_z contains z = max_pooling(in)
            // Now we need to calculate d(L) / d(z) = [d(a) / d(z)] * [d(L) / d(a)]
            // d(L) / d(z) is computed in the next layer, contained in next_layer_data
            // The Jacobian matrix J = d(a) / d(z) is determined by the activation function
            Matrix dLz = m_z;
            Activation::apply_jacobian(m_z, m_a, next_layer_data, dLz);

            // d(L) / d(in_i) = sum_j{ [d(z_j) / d(in_i)] * [d(L) / d(z_j)] }
            // d(z_j) / d(in_i) = 1 if in_i is used to compute z_j and is the maximum
            //                  = 0 otherwise
            m_din.resize(nobs, this->inputSize);
            m_din = 0;

            const int dLz_size = blaze::size(dLz);
            const Scalar* dLz_data = dLz.data();
            const int* loc_data = m_loc.data();
            Scalar* din_data = m_din.data();

	        for (int i = 0; i < dLz_size; i++) {
		        din_data[loc_data[i]] += dLz_data[i];
	        }
        }

        const Matrix& backprop_data() const
        {
            return m_din;
        }

        void update(Optimizer<Scalar>& opt) {}

		std::vector<std::vector<Scalar>> getParameters() const
		{
            return std::vector<std::vector<Scalar>>();
        }

		void setParameters(const std::vector<std::vector<Scalar>>& parameters) {};

        std::vector<Scalar> get_derivatives() const
        {
            return std::vector<Scalar>();
        }
};


} // namespace dnn
} // namespace metric


#endif /* LAYER_MAXPOOLING_H_ */
