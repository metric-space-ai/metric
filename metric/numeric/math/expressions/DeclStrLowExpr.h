// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DECLSTRLOWEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DECLSTRLOWEXPR_H
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
/*!\brief Base class for all declstrlow expression templates.
// \ingroup math
//
// The DeclStrLowExpr class serves as a tag for all expression templates that represent an
// explicit strictly lower declaration (declstrlow) operation. All classes, that represent a
// declstrlow operation and that are used within the expression template environment of the
// Metric numeric library have to derive publicly from this class in order to qualify as declstrlow
// expression template. Only in case a class is derived publicly from the DeclStrLowExpr
// base class, the IsDeclStrLowExpr type trait recognizes the class as valid declstrlow
// expression template.
*/
template <typename MT> // Matrix base type of the expression
struct DeclStrLowExpr : public DeclExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
