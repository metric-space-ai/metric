// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_GENEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_GENEXPR_H
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
// The GenExpr class serves as a tag for all expression templates that represent a generator
// operation. All classes, that represent a generator operation and that are used within the
// expression template environment of the Metric numeric library have to derive publicly from this
// class in order to qualify as generator expression template. Only in case a class is
// derived publicly from the GenExpr base class, the IsGenExpr type trait recognizes the
// class as valid generator expression template.
*/
template <typename T> // Base type of the expression
struct GenExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
