// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SUBEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SUBEXPR_H
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
/*!\brief Base class for all subtraction expression templates.
// \ingroup math
//
// The SubExpr class serves as a tag for all expression templates that implement mathematical
// subtractions. All classes, that represent a mathematical subtraction (vector subtractions
// and matrix subtractions) and that are used within the expression template environment of
// the Metric numeric library have to derive publicly from this class in order to qualify as subtraction
// expression template. Only in case a class is derived publicly from the SubExpr base class,
// the IsSubExpr type trait recognizes the class as valid subtraction expression template.
*/
template <typename T> // Base type of the expression
struct SubExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
