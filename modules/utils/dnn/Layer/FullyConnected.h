#ifndef LAYER_FULLYCONNECTED_H_
#define LAYER_FULLYCONNECTED_H_


#include <iostream>
#include <stdexcept>

#include "../../../../3rdparty/blaze/Math.h"

#include "../Layer.h"
#include "../Utils/Random.h"

namespace MiniDNN
{


///
/// \ingroup Layers
///
/// Fully connected hidden layer
///
template <typename Scalar, typename Activation>
class FullyConnected: public Layer<Scalar>
{
    public:
    
        class LayerSerialProxy : public Layer<Scalar>::LayerSerialProxy {
        public:
            LayerSerialProxy() : Layer<Scalar>::LayerSerialProxy() {}
            LayerSerialProxy(Layer<Scalar>* save_layer) : Layer<Scalar>::LayerSerialProxy(save_layer) {}

            template<class Archive>
            void load(Archive & ar)
            {
                int in_size;
                int out_size;

                ar(CEREAL_NVP(in_size),
                   CEREAL_NVP(out_size));

                Layer<Scalar>::LayerSerialProxy::load_layer = std::make_shared<FullyConnected<Scalar,
                    Activation>>(in_size,
                                 out_size);
            }

            template<class Archive>
            void save(Archive & ar) const
            {
                FullyConnected<Scalar, Activation>* sl = dynamic_cast<FullyConnected<Scalar, Activation>*>(Layer<Scalar>::LayerSerialProxy::save_layer);

                int in_size = sl->in_size();
                int out_size = sl->out_size();

                ar(CEREAL_NVP(in_size),
                   CEREAL_NVP(out_size));
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
		using ConstAlignedMapVec = const blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;
		using AlignedMapVec = blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;

		Matrix m_weight;  // Weight parameters, W(in_size x out_size)
        Vector m_bias;    // Bias parameters, b(out_size x 1)
        Matrix m_dw;      // Derivative of weights
        Vector m_db;      // Derivative of bias
        Matrix m_z;       // Linear term, z = W' * in + b
        Matrix m_a;       // Output of this layer, a = act(z)
        Matrix m_din;     // Derivative of the input of this layer.
        // Note that input of this layer is also the output of previous layer

    public:
	///
        /// Constructor
        ///
        /// \param in_size  Number of input units.
        /// \param out_size Number of output units.
        ///
        FullyConnected(const int in_size, const int out_size) :
            Layer<Scalar>(in_size, out_size)
        {}

        void init(const Scalar& mu, const Scalar& sigma, RNG& rng)
        {
            m_weight.resize(this->m_in_size, this->m_out_size);
            m_bias.resize(this->m_out_size);
            m_dw.resize(this->m_in_size, this->m_out_size);
            m_db.resize(this->m_out_size);
            // Set random coefficients
            internal::set_normal_random(m_weight.data(), blaze::size(m_weight), rng, mu, sigma);
            internal::set_normal_random(m_bias.data(), m_bias.size(), rng, mu, sigma);

            //m_weight = 1;
            //m_bias = 0.01;
        }

        void initConstant(const Scalar weightsValue, const Scalar biasesValue)
        {
	        m_weight.resize(this->m_in_size, this->m_out_size);
	        m_bias.resize(this->m_out_size);
	        m_dw.resize(this->m_in_size, this->m_out_size);
	        m_db.resize(this->m_out_size);

	        m_weight = weightsValue;
	        m_bias = biasesValue;
        }

	// prev_layer_data: in_size x nobs
        void forward(const Matrix& prev_layer_data)
        {
            const int nobs = prev_layer_data.columns();
            // Linear term z = W' * in + b
            m_z.resize(this->m_out_size, nobs);
            /* noalises */
            m_z = blaze::trans(m_weight) * prev_layer_data;
            for (size_t i = 0UL; i < m_z.columns(); i++) {
				blaze::column(m_z, i) += m_bias;
            }
            // Apply activation function
            m_a.resize(this->m_out_size, nobs);
	        //std::cout << blaze::submatrix<0, 0, 5, 20>(prev_layer_data) << std::endl;
	        //std::cout << blaze::submatrix<0, 0, 5, 20>(m_z) << std::endl;
            Activation::activate(m_z, m_a);
        }

        const Matrix& output() const
        {
            return m_a;
        }

        // prev_layer_data: in_size x nobs
        // next_layer_data: out_size x nobs
        void backprop(const Matrix& prev_layer_data, const Matrix& next_layer_data)
        {
            const int nobs = prev_layer_data.columns();
            // After forward stage, m_z contains z = W' * in + b
            // Now we need to calculate d(L) / d(z) = [d(a) / d(z)] * [d(L) / d(a)]
            // d(L) / d(a) is computed in the next layer, contained in next_layer_data
            // The Jacobian matrix J = d(a) / d(z) is determined by the activation function
            Matrix& dLz = m_z;
            Activation::apply_jacobian(m_z, m_a, next_layer_data, dLz);
            // Now dLz contains d(L) / d(z)
            // Derivative for weights, d(L) / d(W) = [d(L) / d(z)] * in'
	        /* noalises */
	        m_dw = prev_layer_data * blaze::trans(dLz) / nobs;
            // Derivative for bias, d(L) / d(b) = d(L) / d(z)
	        /* noalises */
	        m_db = blaze::mean<blaze::rowwise>(dLz);
            // Compute d(L) / d_in = W * [d(L) / d(z)]
            m_din.resize(this->m_in_size, nobs);
	        /* noalises */
	        m_din = m_weight * dLz;
        }

        const Matrix& backprop_data() const
        {
            return m_din;
        }

        void update(Optimizer<Scalar>& opt)
        {
            ConstAlignedMapVec dw(m_dw.data(), blaze::size(m_dw));
            ConstAlignedMapVec db(m_db.data(), m_db.size());
            AlignedMapVec      w(m_weight.data(), blaze::size(m_weight));
            AlignedMapVec      b(m_bias.data(), m_bias.size());
            opt.update(dw, w);
            opt.update(db, b);
        }

        std::vector<Scalar> get_parameters() const
        {
            std::vector<Scalar> res(blaze::size(m_weight) + m_bias.size());
            // Copy the data of weights and bias to a long vector
            std::copy(m_weight.data(), m_weight.data() + blaze::size(m_weight), res.begin());
            std::copy(m_bias.data(), m_bias.data() + m_bias.size(),
                      res.begin() + blaze::size(m_weight));
            return res;
        }

        void set_parameters(const std::vector<Scalar>& param)
        {
            if (static_cast<int>(param.size()) != blaze::size(m_weight) + m_bias.size())
            {
                throw std::invalid_argument("Parameter size does not match");
            }

            std::copy(param.begin(), param.begin() + blaze::size(m_weight), m_weight.data());
            std::copy(param.begin() + blaze::size(m_weight), param.end(), m_bias.data());
        }

        std::vector<Scalar> get_derivatives() const
        {
            std::vector<Scalar> res(blaze::size(m_dw) + m_db.size());
            // Copy the data of weights and bias to a long vector
            std::copy(m_dw.data(), m_dw.data() + blaze::size(m_dw), res.begin());
            std::copy(m_db.data(), m_db.data() + m_db.size(), res.begin() + blaze::size(m_dw));
            return res;
        }
};


} // namespace MiniDNN


#endif /* LAYER_FULLYCONNECTED_H_ */
