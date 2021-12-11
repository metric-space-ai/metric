#ifndef ACTIVATION_IDENTITY_H_
#define ACTIVATION_IDENTITY_H_

#include "blaze/Math.h"

namespace metric
{
namespace dnn
{


///
/// \defgroup Activations Activation Functions
///

///
/// \ingroup Activations
///
/// The identity activation function
///
template<typename Scalar>
class Identity
{
	private:
		using Matrix = blaze::DynamicMatrix<Scalar>;
		using ColumnMatrix = blaze::DynamicMatrix<Scalar, blaze::columnMajor>;
public:
		static std::string getType()
		{
			return "Identity";
		}

        // a = activation(z) = z
        // Z = [z1, ..., zn], A = [a1, ..., an], n observations
        static inline void activate(const Matrix& Z, Matrix& A)
        {
            A = Z;
        }

        // Apply the Jacobian matrix J to a vector f
        // J = d_a / d_z = I
        // g = J * f = f
        // Z = [z1, ..., zn], G = [g1, ..., gn], F = [f1, ..., fn]
        // Note: When entering this function, Z and G may point to the same matrix
        static inline void apply_jacobian(const Matrix& Z, const Matrix& A,
                                          const Matrix& F, ColumnMatrix& G)
        {
            G = F;
        }

		static inline void apply_jacobian(const Matrix& Z, const Matrix& A,
										  const Matrix& F, Matrix& G)
		{
			G = F;
		}
};


	} // namespace dnn
} // namespace metric


#endif /* ACTIVATION_IDENTITY_H_ */
