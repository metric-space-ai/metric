// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DECLEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DECLEXPR_H
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
/*!\brief Base class for all matrix for-each expression templates.
// \ingroup math
//
// The DeclExpr class serves as a tag for all expression templates that represent an explicit
// declaration operation. All classes, that represent a declaration operation and that are used
// within the expression template environment of the Metric numeric library have to derive publicly from
// this class in order to qualify as declaration expression template. Only in case a class is
// derived publicly from the DeclExpr base class, the IsDeclExpr type trait recognizes the class
// as valid declaration expression template.
*/
template <typename T> // Base type of the expression
struct DeclExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
