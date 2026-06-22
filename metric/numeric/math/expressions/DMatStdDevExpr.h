// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DMATSTDDEVEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DMATSTDDEVEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/DMatMapExpr.h>
#include <metric/numeric/math/expressions/DMatVarExpr.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/util/FunctionTrace.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Computes the standard deviation for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the standard deviation computation.
// \return The standard deviation of the given matrix.
// \exception std::invalid_argument Invalid input matrix.
//
// This function computes the
// <a href="https://en.wikipedia.org/wiki/Standard_deviation">standard deviation</a> for the given
// dense matrix \a dm. Example:

   \code
   using mtrc::numeric::DynamicMatrix;

   DynamicMatrix<int> A{ { 1, 3, 2 }
					   , { 2, 6, 4 }
					   , { 9, 6, 3 } };

   const double s = stddev( A );  // Results in sqrt(6.5)
   \endcode

// In case the size of the given matrix is smaller than 2, a \a std::invalid_argument is thrown.
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) stddev(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return sqrt(var(*dm));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the row-/columnwise standard deviation function for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the standard deviation computation.
// \return The row-/columnwise standard deviation of the given matrix.
// \exception std::invalid_argument Invalid input matrix.
//
// This function computes the row-/columnwise
// <a href="https://en.wikipedia.org/wiki/Standard_deviation">standard deviation</a> for the
// given dense matrix \a dm. In case \a RF is set to \a rowwise, the function returns a column
// vector containing the standard deviation of each row of \a dm. In case \a RF is set to
// \a columnwise, the function returns a row vector containing the standard deviation of each
// column of \a dm. Example:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowVector;

   DynamicMatrix<int> A{ { 1, 3, 2 }
					   , { 2, 6, 4 }
					   , { 9, 6, 3 } };

   DynamicVector<double,columnVector> rs;
   DynamicVector<double,rowVector> cs;

   rs = stddev<rowwise>( A );     // Results in ( 1  2  3 )
   cs = stddev<columnwise>( A );  // Results in ( sqrt(19)  sqrt(3)  1 )
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
inline decltype(auto) stddev(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return sqrt(var<RF>(*dm));
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
