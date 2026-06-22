// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CUSTOMMATRIX_H
#define METRIC_NUMERIC_MATH_CUSTOMMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/AlignmentFlag.h>
#include <metric/numeric/math/DenseMatrix.h>
#include <metric/numeric/math/DynamicVector.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/IdentityMatrix.h>
#include <metric/numeric/math/ZeroMatrix.h>
#include <metric/numeric/math/constraints/Scalar.h>
#include <metric/numeric/math/dense/CustomMatrix.h>
#include <metric/numeric/math/dense/DynamicMatrix.h>
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
/*!\brief Specialization of the Rand class template for CustomMatrix.
// \ingroup random
//
// This specialization of the Rand class creates random instances of CustomMatrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  bool SO // Storage order
		  ,
		  typename Tag // Type tag
		  ,
		  typename RT> // Result type
class Rand<CustomMatrix<Type, AF, PF, SO, Tag, RT>> {
  public:
	//*************************************************************************************************
	/*!\brief Randomization of a CustomMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \return void
	*/
	inline void randomize(CustomMatrix<Type, AF, PF, SO, Tag, RT> &matrix) const
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
	/*!\brief Randomization of a CustomMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(CustomMatrix<Type, AF, PF, SO, Tag, RT> &matrix, const Arg &min, const Arg &max) const
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
/*!\brief Setup of a random symmetric CustomMatrix.
//
// \param matrix The matrix to be randomized.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
*/
template <typename Type // Data type of the matrix
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  bool SO // Storage order
		  ,
		  typename Tag // Type tag
		  ,
		  typename RT> // Result type
void makeSymmetric(CustomMatrix<Type, AF, PF, SO, Tag, RT> &matrix)
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
/*!\brief Setup of a random symmetric CustomMatrix.
//
// \param matrix The matrix to be randomized.
// \param min The smallest possible value for a matrix element.
// \param max The largest possible value for a matrix element.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
*/
template <typename Type // Data type of the matrix
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  bool SO // Storage order
		  ,
		  typename Tag // Type tag
		  ,
		  typename RT // Result type
		  ,
		  typename Arg> // Min/max argument type
void makeSymmetric(CustomMatrix<Type, AF, PF, SO, Tag, RT> &matrix, const Arg &min, const Arg &max)
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
/*!\brief Setup of a random Hermitian CustomMatrix.
//
// \param matrix The matrix to be randomized.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
*/
template <typename Type // Data type of the matrix
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  bool SO // Storage order
		  ,
		  typename Tag // Type tag
		  ,
		  typename RT> // Result type
void makeHermitian(CustomMatrix<Type, AF, PF, SO, Tag, RT> &matrix)
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
/*!\brief Setup of a random Hermitian CustomMatrix.
//
// \param matrix The matrix to be randomized.
// \param min The smallest possible value for a matrix element.
// \param max The largest possible value for a matrix element.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
*/
template <typename Type // Data type of the matrix
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  bool SO // Storage order
		  ,
		  typename Tag // Type tag
		  ,
		  typename RT // Result type
		  ,
		  typename Arg> // Min/max argument type
void makeHermitian(CustomMatrix<Type, AF, PF, SO, Tag, RT> &matrix, const Arg &min, const Arg &max)
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
/*!\brief Setup of a random (Hermitian) positive definite CustomMatrix.
//
// \param matrix The matrix to be randomized.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
*/
template <typename Type // Data type of the matrix
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  bool SO // Storage order
		  ,
		  typename Tag // Type tag
		  ,
		  typename RT> // Result type
void makePositiveDefinite(CustomMatrix<Type, AF, PF, SO, Tag, RT> &matrix)
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
