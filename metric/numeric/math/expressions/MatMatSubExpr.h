// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATMATSUBEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATMATSUBEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/SubExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all matrix/matrix subtraction expression templates.
// \ingroup math
//
// The MatMatSubExpr class serves as a tag for all expression templates that implement
// a matrix/matrix subtraction. All classes, that represent a matrix subtraction and that
// are used within the expression template environment of the Metric numeric library have to derive
// publicly from this class in order to qualify as matrix subtraction expression template.
// Only in case a class is derived publicly from the MatMatSubExpr base class, the
// IsMatMatSubExpr type trait recognizes the class as valid matrix subtraction expression
// template.
*/
template <typename MT> // Matrix base type of the expression
struct MatMatSubExpr : public SubExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
