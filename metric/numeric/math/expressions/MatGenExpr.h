// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATGENEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATGENEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/GenExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all matrix generator expression templates.
// \ingroup math
//
// The MatGenExpr class serves as a tag for all expression templates that represent a matrix
// generator operation. All classes, that represent a matrix generator operation and that are
// used within the expression template environment of the Metric numeric library have to derive publicly
// from this class in order to qualify as matrix generator expression template. Only in case a
// class is derived publicly from the MatGenExpr base class, the IsMatGenExpr type trait
// recognizes the class as valid matrix generator expression template.
*/
template <typename MT> // Matrix base type of the expression
struct MatGenExpr : public GenExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
