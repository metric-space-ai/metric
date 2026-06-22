// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_NOSIMDEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_NOSIMDEXPR_H
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
/*!\brief Base class for all no-SIMD expression templates.
// \ingroup math
//
// The NoSIMDExpr class serves as a tag for all expression templates that implement a no-SIMD
// operation. All classes, that represent a no-SIMD operation and that are used within the
// expression template environment of the Metric numeric library have to derive publicly from this class
// in order to qualify as no-SIMD expression template. Only in case a class is derived publicly
// from the NoSIMDExpr base class, the IsNoSIMDExpr type trait recognizes the class as valid
// no-SIMD expression template.
*/
template <typename T> // Base type of the expression
struct NoSIMDExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
