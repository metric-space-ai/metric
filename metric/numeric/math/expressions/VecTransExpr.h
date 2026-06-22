// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECTRANSEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECTRANSEXPR_H
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
/*!\brief Base class for all vector transposition expression templates.
// \ingroup math
//
// The VecTransExpr class serves as a tag for all expression templates that implement a vector
// transposition operation. All classes, that represent a vector transposition operation and
// that are used within the expression template environment of the Metric numeric library have to derive
// publicly from this class in order to qualify as vector transposition expression template. Only
// in case a class is derived publicly from the VecTransExpr base class, the IsVecTransExpr type
// trait recognizes the class as valid vector transposition expression template.
*/
template <typename VT> // Vector base type of the expression
struct VecTransExpr : public TransExpr<VT> {};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Calculating the transpose of a transpose vector.
// \ingroup math
//
// \param vector The vector to be (re-)transposed.
// \return The transpose of the transpose vector.
//
// This function implements a performance optimized treatment of the transpose operation on
// a vector transpose expression. It returns an expression representing the transpose of a
// transpose vector:

   \code
   using mtrc::numeric::columnVector;

   mtrc::numeric::DynamicVector<double,columnVector> a, b;
   // ... Resizing and initialization
   b = trans( trans( a ) );
   \endcode
*/
template <typename VT> // Vector base type of the expression
inline decltype(auto) trans(const VecTransExpr<VT> &vector)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return (*vector).operand();
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
