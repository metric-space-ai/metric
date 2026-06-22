// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DECLUNIUPPEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DECLUNIUPPEXPR_H
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
/*!\brief Base class for all decluniupp expression templates.
// \ingroup math
//
// The DeclUniUppExpr class serves as a tag for all expression templates that represent an
// explicit uniupper declaration (decluniupp) operation. All classes, that represent a
// decluniupp operation and that are used within the expression template environment of the
// Metric numeric library have to derive publicly from this class in order to qualify as decluniupp
// expression template. Only in case class is derived publicly from the DeclUniUppExpr
// base class, the IsDeclUniUppExpr type trait a recognizes the class as valid decluniupp
// expression template.
*/
template <typename MT> // Matrix base type of the expression
struct DeclUniUppExpr : public DeclExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
