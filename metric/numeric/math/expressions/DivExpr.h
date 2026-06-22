// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DIVEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DIVEXPR_H
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
/*!\brief Base class for all division expression templates.
// \ingroup math
//
// The DivExpr class serves as a tag for all expression templates that implement mathematical
// divisions. All classes, that represent a mathematical division (vector/vector divisions,
// vector/scalar divisions and matrix/scalar divisions) and that are used within the expression
// template environment of the Metric numeric library have to derive publicly from this class in order
// to qualify as division expression template. Only in case a class is derived publicly from
// the DivExpr base class, the IsDivExpr type trait recognizes the class as valid division
// expression template.
*/
template <typename T> // Base type of the expression
struct DivExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
