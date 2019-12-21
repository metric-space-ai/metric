#ifndef ACTIVATION_RELU_H_
#define ACTIVATION_RELU_H_

#include "../../../../3rdparty/blaze/Math.h"

namespace metric
{
namespace dnn
{


///
/// \ingroup Activations
///
/// The ReLU activation function
///
template<typename Scalar>
class ReLU
{
	private:
		using Matrix = blaze::DynamicMatrix<Scalar>;
		using ColumnMatrix = blaze::DynamicMatrix<Scalar, blaze::columnMajor>;

	public:
		static std::string getType()
		{
			return "ReLU";
		}

		// a = activation(z) = max(z, 0)
        // Z = [z1, ..., zn], A = [a1, ..., an], n observations
        static inline void activate(const Matrix& Z, Matrix& A)
        {
            //A = Z.array().cwiseMax(Scalar(0));
			for(size_t j = 0UL; j < Z.columns(); j++ ) {
				for(size_t i = 0UL; i < Z.rows(); i++ ) {
		        	if (Z(i, j) < 0) {
				        A(i, j) = 0;
			        } else {
		        		A(i, j) = Z(i, j);
		        	}
		        }
	        }
        }

        // Apply the Jacobian matrix J to a vector f
        // J = d_a / d_z = diag(sign(a)) = diag(a > 0)
        // g = J * f = (a > 0) .* f
        // Z = [z1, ..., zn], G = [g1, ..., gn], F = [f1, ..., fn]
        // Note: When entering this function, Z and G may point to the same matrix
        static inline void apply_jacobian(const Matrix& Z, const Matrix& A,
                                          const Matrix& F, ColumnMatrix& G)
        {
            //G = (A.array() > Scalar(0)).select(F, Scalar(0));
			for(size_t j = 0UL; j < G.columns(); j++ ) {
				for(size_t i = 0UL; i < G.rows(); i++ ) {
			        if (A(i, j) > 0) {
				        G(i, j) = F(i, j);
			        } else {
				        G(i, j) = 0;
			        }
		        }
	        }
        }
};


} // namespace dnn
} // namespace metric


#endif /* ACTIVATION_RELU_H_ */
