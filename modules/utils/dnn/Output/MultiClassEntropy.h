#ifndef OUTPUT_MULTICLASSENTROPY_H_
#define OUTPUT_MULTICLASSENTROPY_H_

#include <stdexcept>


namespace metric
{
namespace dnn
{


///
/// \ingroup Outputs
///
/// Multi-class classification output layer using cross-entropy criterion
///
template <typename Scalar>
class MultiClassEntropy: public Output<Scalar>
{
    private:
		using typename Output<Scalar>::Matrix;
		using typename Output<Scalar>::IntegerVector;

		Matrix m_din;  // Derivative of the input of this layer.
        // Note that input of this layer is also the output of previous layer

    public:
		std::string getType()
		{
			return "MulticlassEntropy";
		}

		void check_target_data(const Matrix& target)
        {
            // Each element should be either 0 or 1
            // Each column has and only has one 1
            const int nobs = target.rows();
            const int nclass = target.columns();

	        for (int i = 0; i < nobs; i++) {
		        int one = 0;

		        for (int j = 0; j < nclass; j++) {
			        if (target(i, j) == Scalar(1)) {
				        one++;
				        continue;
			        }

			        if (target(i, j) != Scalar(0)) {
				        throw std::invalid_argument(
						        "[class MultiClassEntropy]: Target data should only contain zero or one");
			        }
		        }

		        if (one != 1) {
			        throw std::invalid_argument(
					        "[class MultiClassEntropy]: Each column of target data should only contain one \"1\"");
		        }
	        }
        }

        /*void check_target_data(const IntegerVector& target)
        {
            // All elements must be non-negative
            const int nobs = target.rows();

	        for (int i = 0; i < nobs; i++) {
		        if (target(i, 0) < 0) {
			        throw std::invalid_argument("[class MultiClassEntropy]: Target data must be non-negative");
		        }
	        }
        }*/

        // target is a matrix with each column representing an observation
        // Each column is a vector that has a one at some location and has zeros elsewhere
        void evaluate(const Matrix& prev_layer_data, const Matrix& target)
        {
            // Check dimension
            const int nobs = prev_layer_data.rows();
            const int nclass = prev_layer_data.columns();

	        if ((target.rows() != nobs) || (target.columns() != nclass)) {
		        throw std::invalid_argument("[class MultiClassEntropy]: Target data have incorrect dimension");
	        }

            // Compute the derivative of the input of this layer
            // L = -sum(log(phat) * y)
            // in = phat
            // d(L) / d(in) = -y / phat
            m_din.resize(nclass, nobs);
            /* noalias */
            //m_din = -target.cwiseQuotient(prev_layer_data);
        }

        // target is a vector of class labels that take values from [0, 1, ..., nclass - 1]
        // The i-th element of target is the class label for observation i
        void evaluate(const Matrix& prev_layer_data, const IntegerVector& target)
        {
            // Check dimension
            const int nobs = prev_layer_data.rows();
            const int nclass = prev_layer_data.columns();

	        if (target.rows() != nobs) {
		        throw std::invalid_argument("[class MultiClassEntropy]: Target data have incorrect dimension");
	        }

            // Compute the derivative of the input of this layer
            // L = -log(phat[y])
            // in = phat
            // d(L) / d(in) = [0, 0, ..., -1/phat[y], 0, ..., 0]
            m_din.resize(nobs, nclass);
            m_din = 0;

	        for (int i = 0; i < nobs; i++) {
		        m_din(i, target(i, 0)) = -Scalar(1) / prev_layer_data(i, target(i, 0));
	        }
        }

        const Matrix& backprop_data() const
        {
            return m_din;
        }

        Scalar loss() const
        {
            // L = -sum(log(phat) * y)
            // in = phat
            // d(L) / d(in) = -y / phat
            // m_din contains 0 if y = 0, and -1/phat if y = 1
            Scalar res = Scalar(0);
            const int nelem = blaze::size(m_din);
            const Scalar* din_data = m_din.data();

	        for (int i = 0; i < nelem; i++) {
		        if (din_data[i] < Scalar(0)) {
			        res += std::log(-din_data[i]);
		        }
	        }

            return res / m_din.rows();
        }
};


} // namespace dnn
} // namespace metric


#endif /* OUTPUT_MULTICLASSENTROPY_H_ */
