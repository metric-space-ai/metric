// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATEXPEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATEXPEXPR_H
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
/*!\brief Base class for all matrix exponential expression templates.
// \ingroup math
//
// The MatExpExpr class serves as a tag for all expression templates that implement a matrix
// exponential operation. All classes, that represent a matrix exponential operation and that are
// used within the expression template environment of the Metric numeric library have to derive publicly
// from this class in order to qualify as matrix exponential expression template. Only in case
// a class is derived publicly from the MatExpExpr base class, the IsMatExpExpr type trait
// recognizes the class as valid matrix exponential expression template.
*/
template <typename MT> // Matrix base type of the expression
struct MatExpExpr : public Expression<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
