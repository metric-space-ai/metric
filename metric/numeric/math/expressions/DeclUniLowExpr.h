// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DECLUNILOWEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DECLUNILOWEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/DeclExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all declunilow expression templates.
// \ingroup math
//
// The DeclUniLowExpr class serves as a tag for all expression templates that represent an
// explicit unilower declaration (declunilow) operation. All classes, that represent a
// declunilow operation and that are used within the expression template environment of the
// Metric numeric library have to derive publicly from this class in order to qualify as declunilow
// expression template. Only in case a class is derived publicly from the DeclUniLowExpr
// base class, the IsDeclUniLowExpr type trait recognizes the class as valid declunilow
// expression template.
*/
template <typename MT> // Matrix base type of the expression
struct DeclUniLowExpr : public DeclExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
