// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_EXPANDEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_EXPANDEXPR_H
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
/*!\brief Base class for all expansion expression templates.
// \ingroup math
//
// The ExpandExpr class serves as a tag for all expression templates that implement expansion
// operations. All classes, that represent an expansion operation (e.g. vector expansions) and
// that are used within the expression template environment of the Metric numeric library have to derive
// publicly from this class in order to qualify as expansion expression template. Only in case
// a class is derived publicly from the ExpandExpr base class, the IsExpandExpr type trait
// recognizes the class as valid expansion expression template.
*/
template <typename T> // Base type of the expression
struct ExpandExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
