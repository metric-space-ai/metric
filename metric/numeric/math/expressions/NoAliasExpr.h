// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_NOALIASEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_NOALIASEXPR_H
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
/*!\brief Base class for all no-alias expression templates.
// \ingroup math
//
// The NoAliasExpr class serves as a tag for all expression templates that implement a no-alias
// operation. All classes, that represent a no-alias operation and that are used within the
// expression template environment of the Metric numeric library have to derive publicly from this class
// in order to qualify as no-alias expression template. Only in case a class is derived publicly
// from the NoAliasExpr base class, the IsNoAliasExpr type trait recognizes the class as valid
// no-alias expression template.
*/
template <typename T> // Base type of the expression
struct NoAliasExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
