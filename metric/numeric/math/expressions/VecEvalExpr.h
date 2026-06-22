// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECEVALEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECEVALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/EvalExpr.h>
#include <metric/numeric/util/FunctionTrace.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all vector evaluation expression templates.
// \ingroup math
//
// The VecEvalExpr class serves as a tag for all expression templates that implement a vector
// evaluation operation. All classes, that represent a vector evaluation operation and that
// are used within the expression template environment of the Metric numeric library have to derive
// publicly from this class in order to qualify as vector evaluation expression template. Only
// in case a class is derived publicly from the VecEvalExpr base class, the IsVecEvalExpr type
// trait recognizes the class as valid vector evaluation expression template.
*/
template <typename VT> // Vector base type of the expression
struct VecEvalExpr : public EvalExpr<VT> {};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Evaluation of the given vector evaluation expression.
// \ingroup math
//
// \param vector The input evaluation expression.
// \return The evaluated vector.
//
// This function implements a performance optimized treatment of the evaluation of a vector
// evaluation expression.
*/
template <typename VT> // Vector base type of the expression
inline decltype(auto) eval(const VecEvalExpr<VT> &vector)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return *vector;
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
