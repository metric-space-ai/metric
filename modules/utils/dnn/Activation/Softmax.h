#ifndef ACTIVATION_SOFTMAX_H_
#define ACTIVATION_SOFTMAX_H_

namespace metric
{
namespace dnn
{


///
/// \ingroup Activations
///
/// The softmax activation function
///
template<typename Scalar>
class Softmax
{
    private:
        //typedef Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> Matrix;
        //typedef Eigen::Array<Scalar, 1, Eigen::Dynamic> RowArray;
		using Matrix = blaze::DynamicMatrix<Scalar>;
		using Vector = blaze::DynamicVector<Scalar>;
		using RowVector = blaze::DynamicVector<Scalar, blaze::rowVector>;

	public:
		static std::string getType()
		{
			return "Softmax";
		}

		// a = activation(z) = softmax(z)
        // Z = [z1, ..., zn], A = [a1, ..., an], n observations
        static inline void activate(const Matrix& Z, Matrix& A)
        {
        	A = blaze::softmax<blaze::rowwise>(Z);
        }

        // Apply the Jacobian matrix J to a vector f
        // J = d_a / d_z = diag(a) - a * a'
        // g = J * f = a .* f - a * (a' * f) = a .* (f - a'f)
        // Z = [z1, ..., zn], G = [g1, ..., gn], F = [f1, ..., fn]
        // Note: When entering this function, Z and G may point to the same matrix
        static inline void apply_jacobian(const Matrix& Z, const Matrix& A,
                                          const Matrix& F, Matrix& G)
        {
            RowVector a_dot_f = A.cwiseProduct(F).colwise().sum();
            G = A * (F.rowwise() - a_dot_f);
        }
};


} // namespace dnn
} // namespace metric


#endif /* ACTIVATION_SOFTMAX_H_ */
