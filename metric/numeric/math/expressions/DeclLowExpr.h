// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DECLLOWEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DECLLOWEXPR_H
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
/*!\brief Base class for all decllow expression templates.
// \ingroup math
//
// The DeclLowExpr class serves as a tag for all expression templates that represent an explicit
// lower declaration (decllow) operation. All classes, that represent a decllow operation and
// that are used within the expression template environment of the Metric numeric library have to derive
// publicly from this class in order to qualify as decllow expression template. Only in case
// a class is derived publicly from the DeclLowExpr base class, the IsDeclLowExpr type trait
// recognizes the class as valid decllow expression template.
*/
template <typename MT> // Matrix base type of the expression
struct DeclLowExpr : public DeclExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
