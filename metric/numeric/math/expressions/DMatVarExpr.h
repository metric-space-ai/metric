// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DMATVAREXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DMATVAREXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/ReductionFlag.h>
#include <metric/numeric/math/dense/UniformMatrix.h>
#include <metric/numeric/math/dense/UniformVector.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/functors/Pow2.h>
#include <metric/numeric/math/shims/Invert.h>
#include <metric/numeric/math/typetraits/IsUniform.h>
#include <metric/numeric/math/typetraits/UnderlyingBuiltin.h>
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
/*!\brief Backend implementation of the \c var() function for general dense matrices.
// \ingroup dense_matrix
//
// \param dm The given general dense matrix for the variance computation.
// \return The variance of the given matrix.
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) var_backend(const DenseMatrix<MT, SO> &dm, FalseType)
{
	using BT = UnderlyingBuiltin_t<MT>;

	METRIC_NUMERIC_INTERNAL_ASSERT(size(*dm) > 1UL, "Invalid matrix size detected");

	const auto m(uniform<SO>(rows(*dm), columns(*dm), mean(*dm)));

	return sum(map((*dm) - m, Pow2())) * inv(BT(size(*dm) - 1UL));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the \c var() function for uniform dense matrices.
// \ingroup dense_matrix
//
// \param dm The given uniform dense matrix for the variance computation.
// \return The var of the given matrix.
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) var_backend(const DenseMatrix<MT, SO> &dm, TrueType)
{
	MAYBE_UNUSED(dm);

	METRIC_NUMERIC_INTERNAL_ASSERT(size(*dm) > 1UL, "Invalid matrix size detected");

	return ElementType_t<MT>();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the variance for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the variance computation.
// \return The variance of the given matrix.
// \exception std::invalid_argument Invalid input matrix.
//
// This function computes the <a href="https://en.wikipedia.org/wiki/Variance">variance</a> for
// the given dense matrix \a dm. Example:

   \code
   using mtrc::numeric::DynamicMatrix;

   DynamicMatrix<int> A{ { 1, 3, 2 }
					   , { 2, 6, 4 }
					   , { 9, 6, 3 } };

   const double v = var( A );  // Results in 6.5
   \endcode

// In case the size of the given matrix is smaller than 2, a \a std::invalid_argument is thrown.
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) var(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (size(*dm) < 2UL) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid input matrix");
	}

	return var_backend(*dm, IsUniform<MT>());
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the row-/column-wise \c var() function for general dense matrices.
// \ingroup dense_matrix
//
// \param dm The given general dense matrix for the variance computation.
// \return The row-/column-wise variance of the given matrix.
*/
template <ReductionFlag RF // Reduction flag
		  ,
		  typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
decltype(auto) var_backend(const DenseMatrix<MT, SO> &dm, FalseType)
{
	using BT = UnderlyingBuiltin_t<MT>;

	const size_t n(RF == rowwise ? columns(*dm) : rows(*dm));

	METRIC_NUMERIC_INTERNAL_ASSERT(n > 1UL, "Invalid matrix size detected");

	const auto m(expand(mean<RF>(*dm), n));

	return sum<RF>(map((*dm) - m, Pow2())) * inv(BT(n - 1UL));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the row-/column-wise \c var() function for uniform dense matrices.
// \ingroup dense_matrix
//
// \param dm The given general dense matrix for the variance computation.
// \return The row-/column-wise variance of the given matrix.
*/
template <ReductionFlag RF // Reduction flag
		  ,
		  typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
decltype(auto) var_backend(const DenseMatrix<MT, SO> &dm, TrueType)
{
	const size_t n(RF == rowwise ? rows(*dm) : columns(*dm));

	METRIC_NUMERIC_INTERNAL_ASSERT(n > 0UL, "Invalid matrix size detected");

	constexpr bool TF((RF == rowwise ? columnVector : rowVector));

	return uniform<TF>(n, ElementType_t<MT>());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the row-/column-wise variance function for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the variance computation.
// \return The row-/column-wise variance of the given matrix.
// \exception std::invalid_argument Invalid input matrix.
//
// This function computes the row-/column-wise
// <a href="https://en.wikipedia.org/wiki/Variance">variance</a> for the given dense matrix
// \a dm. In case \a RF is set to \a rowwise, the function returns a column vector containing
// the variance of each row of \a dm. In case \a RF is set to \a columnwise, the function
// returns a row vector containing the variance of each column of \a dm. Example:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowVector;

   DynamicMatrix<int> A{ { 1, 3, 2 }
					   , { 2, 6, 4 }
					   , { 9, 6, 3 } };

   DynamicVector<double,columnVector> rv;
   DynamicVector<double,rowVector> cv;

   rv = var<rowwise>( A );     // Results in ( 1  4  9 )
   cv = var<columnwise>( A );  // Results in ( 19  3  1 )
   \endcode

// In case \a RF is set to \a rowwise and the number of columns of the given matrix is smaller
// than 2 or in case \a RF is set to \a columnwise and the number of rows of the given matrix is
// smaller than 2, a \a std::invalid_argument is thrown.
*/
template <ReductionFlag RF // Reduction flag
		  ,
		  typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) var(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_STATIC_ASSERT_MSG(RF < 2UL, "Invalid reduction flag");

	const size_t n(RF == rowwise ? columns(*dm) : rows(*dm));

	if (n < 2UL) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid input matrix");
	}

	return var_backend<RF>(*dm, IsUniform<MT>());
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
