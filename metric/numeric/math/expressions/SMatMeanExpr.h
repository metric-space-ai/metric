// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SMATMEANEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SMATMEANEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/ReductionFlag.h>
#include <metric/numeric/math/dense/UniformVector.h>
#include <metric/numeric/math/expressions/SparseMatrix.h>
#include <metric/numeric/math/shims/Invert.h>
#include <metric/numeric/math/typetraits/IsZero.h>
#include <metric/numeric/math/typetraits/UnderlyingBuiltin.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/StaticAssert.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the \c mean() function for general sparse matrices.
// \ingroup sparse_matrix
//
// \param sm The given general sparse matrix for the mean computation.
// \return The mean of the given matrix.
*/
template <typename MT // Type of the sparse matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) mean_backend(const SparseMatrix<MT, SO> &sm, FalseType)
{
	using BT = UnderlyingBuiltin_t<MT>;

	METRIC_NUMERIC_INTERNAL_ASSERT(size(*sm) > 0UL, "Invalid matrix size detected");

	return sum(*sm) * inv(BT(size(*sm)));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the \c mean() function for uniform sparse matrices.
// \ingroup sparse_matrix
//
// \param sm The given uniform sparse matrix for the mean computation.
// \return The mean of the given matrix.
*/
template <typename MT // Type of the sparse matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) mean_backend(const SparseMatrix<MT, SO> &sm, TrueType)
{
	MAYBE_UNUSED(sm);

	METRIC_NUMERIC_INTERNAL_ASSERT(size(*sm) > 0UL, "Invalid matrix size detected");

	return ElementType_t<MT>();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the (arithmetic) mean for the given sparse matrix.
// \ingroup sparse_matrix
//
// \param sm The given sparse matrix for the mean computation.
// \return The mean of the given matrix.
// \exception std::invalid_argument Invalid input matrix.
//
// This function computes the
// <a href="https://en.wikipedia.org/wiki/Arithmetic_mean">(arithmetic) mean</a> for the given
// sparse matrix \a sm. Both the non-zero and zero elements of the sparse matrix are taken into
// account. Example:

   \code
   using mtrc::numeric::CompressedMatrix;

   CompressedMatrix<int> A{ { 1, 3, 2 }
						  , { 2, 6, 4 }
						  , { 9, 6, 3 } };

   const double m = mean( A );  // Results in 4 (i.e. 36/9)
   \endcode

// In case the number of rows or columns of the given matrix is 0, a \a std::invalid_argument is
// thrown.
*/
template <typename MT // Type of the sparse matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) mean(const SparseMatrix<MT, SO> &sm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (size(*sm) == 0UL) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid input matrix");
	}

	return mean_backend(*sm, IsZero<MT>());
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the row-/column-wise \c mean() function for general sparse
//        matrices.
// \ingroup sparse_matrix
//
// \param sm The given general sparse matrix for the mean computation.
// \return The row-/column-wise mean of the given matrix.
*/
template <ReductionFlag RF // Reduction flag
		  ,
		  typename MT // Type of the sparse matrix
		  ,
		  bool SO> // Storage order
decltype(auto) mean_backend(const SparseMatrix<MT, SO> &sm, FalseType)
{
	using BT = UnderlyingBuiltin_t<MT>;

	const size_t n(RF == rowwise ? columns(*sm) : rows(*sm));

	METRIC_NUMERIC_INTERNAL_ASSERT(n > 0UL, "Invalid matrix size detected");

	return sum<RF>(*sm) * inv(BT(n));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the row-/column-wise \c mean() function for uniform sparse
//        matrices.
// \ingroup sparse_matrix
//
// \param sm The given general sparse matrix for the mean computation.
// \return The row-/column-wise mean of the given matrix.
*/
template <ReductionFlag RF // Reduction flag
		  ,
		  typename MT // Type of the sparse matrix
		  ,
		  bool SO> // Storage order
decltype(auto) mean_backend(const SparseMatrix<MT, SO> &sm, TrueType)
{
	const size_t n(RF == rowwise ? rows(*sm) : columns(*sm));

	METRIC_NUMERIC_INTERNAL_ASSERT(n > 0UL, "Invalid matrix size detected");

	constexpr bool TF((RF == rowwise ? columnVector : rowVector));

	return uniform<TF>(n, ElementType_t<MT>());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the row-/columnwise mean function for the given sparse matrix.
// \ingroup sparse_matrix
//
// \param sm The given sparse matrix for the mean computation.
// \return The row-/columnwise mean of the given matrix.
// \exception std::invalid_argument Invalid input matrix.
//
// This function computes the row-/columnwise
// <a href="https://en.wikipedia.org/wiki/Arithmetic_mean">(arithmetic) mean</a> for the given
// sparse matrix \a sm. In case \a RF is set to \a rowwise, the function returns a column vector
// containing the mean of each row of \a sm. In case \a RF is set to \a columnwise, the function
// returns a row vector containing the mean of each column of \a sm. Both the non-zero and zero
// elements of the sparse matrix are taken into account. Example:

   \code
   using mtrc::numeric::CompressedMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowVector;

   CompressedMatrix<int> A{ { 1, 3, 2 }
						  , { 2, 6, 4 }
						  , { 9, 6, 3 } };

   DynamicVector<double,columnVector> rm;
   DynamicVector<double,rowVector> cm;

   rm = mean<rowwise>( A );     // Results in ( 2  4  6 )
   cm = mean<columnwise>( A );  // Results in ( 4  5  3 )
   \endcode

// In case \a RF is set to \a rowwise and the number of columns of the given matrix is 0 or in
// case \a RF is set to \a columnwise and the number of rows of the given matrix is 0, a
// \a std::invalid_argument is thrown.
*/
template <ReductionFlag RF // Reduction flag
		  ,
		  typename MT // Type of the sparse matrix
		  ,
		  bool SO> // Storage order
decltype(auto) mean(const SparseMatrix<MT, SO> &sm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_STATIC_ASSERT_MSG(RF < 2UL, "Invalid reduction flag");

	const size_t n(RF == rowwise ? columns(*sm) : rows(*sm));

	if (n == 0UL) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid input matrix");
	}

	return mean_backend<RF>(*sm, IsZero<MT>());
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
