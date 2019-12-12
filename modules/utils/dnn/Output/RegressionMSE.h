#ifndef OUTPUT_REGRESSIONMSE_H_
#define OUTPUT_REGRESSIONMSE_H_

#include <stdexcept>

namespace dnn
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
		using Matrix = blaze::DynamicMatrix<Scalar, blaze::columnMajor>;
        Matrix m_din;  // Derivative of the input of this layer.
        // Note that input of this layer is also the output of previous layer

    public:
        void evaluate(const Matrix& prev_layer_data, const Matrix& target)
        {
            // Check dimension
            const int nobs = prev_layer_data.columns();
            const int nvar = prev_layer_data.rows();

            if ((target.columns() != nobs) || (target.rows() != nvar))
            {
                throw std::invalid_argument("[class RegressionMSE]: Target data have incorrect dimension");
            }

            // Compute the derivative of the input of this layer
            // L = 0.5 * ||yhat - y||^2
            // in = yhat
            // d(L) / d(in) = yhat - y
            m_din.resize(nvar, nobs);
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
            return blaze::sqrNorm(m_din) / m_din.columns() * Scalar(0.5);
        }
};


} // namespace dnn


#endif /* OUTPUT_REGRESSIONMSE_H_ */
