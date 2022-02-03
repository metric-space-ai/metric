#ifndef ACTIVATION_SIGMOID_H_
#define ACTIVATION_SIGMOID_H_

#include <blaze/Math.h>

namespace metric {
namespace dnn {

///
/// \ingroup Activations
///
/// The sigmoid activation function
///
template <typename Scalar> class Sigmoid {
  private:
	using Matrix = blaze::DynamicMatrix<Scalar>;
	using ColumnMatrix = blaze::DynamicMatrix<Scalar, blaze::columnMajor>;

  public:
	static std::string getType() { return "Sigmoid"; }

	// a = activation(z) = 1 / (1 + exp(-z))
	// Z = [z1, ..., zn], A = [a1, ..., an], n observations
	static inline void activate(const Matrix &Z, Matrix &A) { A = Scalar(1) / (Scalar(1) + blaze::exp(-Z)); }

	// Apply the Jacobian matrix J to a vector f
	// J = d_a / d_z = diag(a .* (1 - a))
	// g = J * f = a .* (1 - a) .* f
	// Z = [z1, ..., zn], G = [g1, ..., gn], F = [f1, ..., fn]
	// Note: When entering this function, Z and G may point to the same matrix
	static inline void apply_jacobian(const Matrix &Z, const Matrix &A, const Matrix &F, ColumnMatrix &G)
	{
		G = A % (Scalar(1) - A) % F;
	}
};

} // namespace dnn
} // namespace metric

#endif /* ACTIVATION_SIGMOID_H_ */
