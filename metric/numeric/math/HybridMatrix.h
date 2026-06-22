// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_HYBRIDMATRIX_H
#define METRIC_NUMERIC_MATH_HYBRIDMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/DenseMatrix.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/HybridVector.h>
#include <metric/numeric/math/IdentityMatrix.h>
#include <metric/numeric/math/StaticMatrix.h>
#include <metric/numeric/math/ZeroMatrix.h>
#include <metric/numeric/math/constraints/Scalar.h>
#include <metric/numeric/math/dense/HybridMatrix.h>
#include <metric/numeric/math/shims/Conjugate.h>
#include <metric/numeric/math/shims/Real.h>
#include <metric/numeric/math/typetraits/UnderlyingBuiltin.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/Random.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  RAND SPECIALIZATION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for HybridMatrix.
// \ingroup random
//
// This specialization of the Rand class creates random instances of HybridMatrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
class Rand<HybridMatrix<Type, M, N, SO, AF, PF, Tag>> {
  public:
	//*************************************************************************************************
	/*!\brief Generation of a random HybridMatrix.
	//
	// \return The generated random matrix.
	*/
	inline const HybridMatrix<Type, M, N, SO, AF, PF, Tag> generate(size_t m, size_t n) const
	{
		HybridMatrix<Type, M, N, SO, AF, PF, Tag> matrix(m, n);
		randomize(matrix);
		return matrix;
	}
	//*************************************************************************************************

	//*************************************************************************************************
	/*!\brief Generation of a random HybridMatrix.
	//
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return The generated random matrix.
	*/
	template <typename Arg> // Min/max argument type
	inline const HybridMatrix<Type, M, N, SO, AF, PF, Tag> generate(size_t m, size_t n, const Arg &min,
																	const Arg &max) const
	{
		HybridMatrix<Type, M, N, SO, AF, PF, Tag> matrix(m, n);
		randomize(matrix, min, max);
		return matrix;
	}
	//*************************************************************************************************

	//*************************************************************************************************
	/*!\brief Randomization of a HybridMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \return void
	*/
	inline void randomize(HybridMatrix<Type, M, N, SO, AF, PF, Tag> &matrix) const
	{
		using mtrc::numeric::randomize;

		const size_t m(matrix.rows());
		const size_t n(matrix.columns());

		for (size_t i = 0UL; i < m; ++i) {
			for (size_t j = 0UL; j < n; ++j) {
				randomize(matrix(i, j));
			}
		}
	}
	//*************************************************************************************************

	//*************************************************************************************************
	/*!\brief Randomization of a HybridMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(HybridMatrix<Type, M, N, SO, AF, PF, Tag> &matrix, const Arg &min, const Arg &max) const
	{
		using mtrc::numeric::randomize;

		const size_t m(matrix.rows());
		const size_t n(matrix.columns());

		for (size_t i = 0UL; i < m; ++i) {
			for (size_t j = 0UL; j < n; ++j) {
				randomize(matrix(i, j), min, max);
			}
		}
	}
	//*************************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  MAKE FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Setup of a random symmetric HybridMatrix.
//
// \param matrix The matrix to be randomized.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
void makeSymmetric(HybridMatrix<Type, M, N, SO, AF, PF, Tag> &matrix)
{
	using mtrc::numeric::randomize;

	if (!isSquare(*matrix)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid non-square matrix provided");
	}

	const size_t n(matrix.rows());

	for (size_t i = 0UL; i < n; ++i) {
		for (size_t j = 0UL; j < i; ++j) {
			randomize(matrix(i, j));
			matrix(j, i) = matrix(i, j);
		}
		randomize(matrix(i, i));
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isSymmetric(matrix), "Non-symmetric matrix detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Setup of a random symmetric HybridMatrix.
//
// \param matrix The matrix to be randomized.
// \param min The smallest possible value for a matrix element.
// \param max The largest possible value for a matrix element.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag // Type tag
		  ,
		  typename Arg> // Min/max argument type
void makeSymmetric(HybridMatrix<Type, M, N, SO, AF, PF, Tag> &matrix, const Arg &min, const Arg &max)
{
	using mtrc::numeric::randomize;

	if (!isSquare(*matrix)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid non-square matrix provided");
	}

	const size_t n(matrix.rows());

	for (size_t i = 0UL; i < n; ++i) {
		for (size_t j = 0UL; j < i; ++j) {
			randomize(matrix(i, j), min, max);
			matrix(j, i) = matrix(i, j);
		}
		randomize(matrix(i, i), min, max);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isSymmetric(matrix), "Non-symmetric matrix detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Setup of a random Hermitian HybridMatrix.
//
// \param matrix The matrix to be randomized.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
void makeHermitian(HybridMatrix<Type, M, N, SO, AF, PF, Tag> &matrix)
{
	using mtrc::numeric::randomize;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SCALAR_TYPE(Type);

	using BT = UnderlyingBuiltin_t<Type>;

	if (!isSquare(*matrix)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid non-square matrix provided");
	}

	const size_t n(matrix.rows());

	for (size_t i = 0UL; i < n; ++i) {
		for (size_t j = 0UL; j < i; ++j) {
			randomize(matrix(i, j));
			matrix(j, i) = conj(matrix(i, j));
		}
		matrix(i, i) = rand<BT>();
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isHermitian(matrix), "Non-Hermitian matrix detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Setup of a random Hermitian HybridMatrix.
//
// \param matrix The matrix to be randomized.
// \param min The smallest possible value for a matrix element.
// \param max The largest possible value for a matrix element.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag // Type tag
		  ,
		  typename Arg> // Min/max argument type
void makeHermitian(HybridMatrix<Type, M, N, SO, AF, PF, Tag> &matrix, const Arg &min, const Arg &max)
{
	using mtrc::numeric::randomize;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SCALAR_TYPE(Type);

	using BT = UnderlyingBuiltin_t<Type>;

	if (!isSquare(*matrix)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid non-square matrix provided");
	}

	const size_t n(matrix.rows());

	for (size_t i = 0UL; i < n; ++i) {
		for (size_t j = 0UL; j < i; ++j) {
			randomize(matrix(i, j), min, max);
			matrix(j, i) = conj(matrix(i, j));
		}
		matrix(i, i) = rand<BT>(real(min), real(max));
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isHermitian(matrix), "Non-Hermitian matrix detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Setup of a random (Hermitian) positive definite HybridMatrix.
//
// \param matrix The matrix to be randomized.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
void makePositiveDefinite(HybridMatrix<Type, M, N, SO, AF, PF, Tag> &matrix)
{
	using mtrc::numeric::randomize;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SCALAR_TYPE(Type);

	if (!isSquare(*matrix)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid non-square matrix provided");
	}

	const size_t n(matrix.rows());

	randomize(matrix);
	matrix *= ctrans(matrix);

	for (size_t i = 0UL; i < n; ++i) {
		matrix(i, i) += Type(n);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isHermitian(matrix), "Non-symmetric matrix detected");
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
