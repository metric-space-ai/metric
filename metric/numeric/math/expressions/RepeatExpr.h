// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_REPEATEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_REPEATEXPR_H
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
/*!\brief Base class for all for-each expression templates.
// \ingroup math
//
// The RepeatExpr class serves as a tag for all expression templates that represent a repeat
// operation. All classes, that represent a repeat operation and that are used within the
// expression template environment of the Metric numeric library have to derive publicly from this
// class in order to qualify as repeater expression template. Only in case a class is derived
// publicly from the RepeatExpr base class, the IsRepeatExpr type trait recognizes the class
// as valid repeater expression template.
*/
template <typename T> // Base type of the expression
struct RepeatExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
