// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_EVALEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_EVALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Expression.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all evaluation expression templates.
// \ingroup math
//
// The EvalExpr class serves as a tag for all expression templates that implement an evaluation
// operation. All classes, that represent an evaluation operation and that are used within the
// expression template environment of the Metric numeric library have to derive publicly from this class
// in order to qualify as evaluation expression template. Only in case a class is derived
// publicly from the EvalExpr base class, the IsEvalExpr type trait recognizes the class as
// valid evaluation expression template.
*/
template <typename T> // Base type of the expression
struct EvalExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
