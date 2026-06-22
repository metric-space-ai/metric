// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATEVALEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATEVALEXPR_H
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
/*!\brief Base class for all matrix evaluation expression templates.
// \ingroup math
//
// The MatEvalExpr class serves as a tag for all expression templates that implement a matrix
// evaluation operation. All classes, that represent a matrix evaluation operation and that
// are used within the expression template environment of the Metric numeric library have to derive
// publicly from this class in order to qualify as matrix evaluation expression template. Only
// in case a class is derived publicly from the MatEvalExpr base class, the IsMatEvalExpr type
// trait recognizes the class as valid matrix evaluation expression template.
*/
template <typename MT> // Matrix base type of the expression
struct MatEvalExpr : public EvalExpr<MT> {};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Evaluation of the given matrix evaluation expression.
// \ingroup math
//
// \param matrix The input evaluation expression.
// \return The evaluated matrix.
//
// This function implements a performance optimized treatment of the evaluation of a matrix
// evaluation expression.
*/
template <typename MT> // Matrix base type of the expression
inline decltype(auto) eval(const MatEvalExpr<MT> &matrix)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return *matrix;
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
