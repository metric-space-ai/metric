// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_CROSSEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_CROSSEXPR_H
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
/*!\brief Base class for all cross product expression templates.
// \ingroup math
//
// The CrossExpr class serves as a tag for all expression templates that implement mathematical
// cross products. All classes, that represent a mathematical cross product and that are used
// within the expression template environment of the Metric numeric library have to derive publicly from
// this class in order to qualify as cross product expression template. Only in case a class is
// derived publicly from the CrossExpr base class, the IsCrossExpr type trait recognizes the
// class as valid cross product expression template.
*/
template <typename VT> // Vector base type of the expression
struct CrossExpr : public Expression<VT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
