#ifndef OUTPUT_REGRESSIONMSE_H_
#define OUTPUT_REGRESSIONMSE_H_

#include <stdexcept>

namespace MiniDNN
{


///
/// \ingroup Outputs
///
/// Regression output layer using Mean Squared Error (MSE) criterion
///
template <typename Scalar>
class RegressionMSE: public Output<Scalar>
{
    private:
		using Matrix = blaze::DynamicMatrix<Scalar>;
        Matrix m_din;  // Derivative of the input of this layer.
        // Note that input of this layer is also the output of previous layer

    public:
        void evaluate(const Matrix& prev_layer_data, const Matrix& target)
        {
            // Check dimension
            const int nobs = prev_layer_data.rows();
            const int nvar = prev_layer_data.columns();

            if ((target.rows() != nobs) || (target.columns() != nvar))
            {
                throw std::invalid_argument("[class RegressionMSE]: Target data have incorrect dimension");
            }

            // Compute the derivative of the input of this layer
            // L = 0.5 * ||yhat - y||^2
            // in = yhat
            // d(L) / d(in) = yhat - y
            m_din.resize(nobs, nvar);
            //noalises
            m_din = prev_layer_data - target;
        }

        const Matrix& backprop_data() const
        {
            return m_din;
        }

        Scalar loss() const
        {
            // L = 0.5 * ||yhat - y||^2
            return blaze::sqrNorm(m_din) / m_din.rows() * Scalar(0.5);
        }
};


} // namespace MiniDNN


#endif /* OUTPUT_REGRESSIONMSE_H_ */
