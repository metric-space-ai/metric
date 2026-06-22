// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATTRANSEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATTRANSEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/TransExpr.h>
#include <metric/numeric/util/FunctionTrace.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all matrix transposition expression templates.
// \ingroup math
//
// The MatTransExpr class serves as a tag for all expression templates that implement a matrix
// transposition operation. All classes, that represent a matrix transposition operation and
// that are used within the expression template environment of the Metric numeric library have to derive
// publicly from this class in order to qualify as matrix transposition expression template. Only
// in case a class is derived publicly from the MatTransExpr base class, the IsMatTransExpr type
// trait recognizes the class as valid matrix transposition expression template.
*/
template <typename MT> // Matrix base type of the expression
struct MatTransExpr : public TransExpr<MT> {};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Calculating the transpose of a transpose matrix.
// \ingroup math
//
// \param matrix The matrix to be (re-)transposed.
// \return The transpose of the transpose matrix.
//
// This function implements a performance optimized treatment of the transpose operation on
// a matrix transpose expression. It returns an expression representing the transpose of a
// transpose matrix:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A, B;
   // ... Resizing and initialization
   B = trans( trans( A ) );
   \endcode
*/
template <typename MT> // Matrix base type of the expression
inline decltype(auto) trans(const MatTransExpr<MT> &matrix)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return (*matrix).operand();
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
