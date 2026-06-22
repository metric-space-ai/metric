// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DECLSTRUPPEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DECLSTRUPPEXPR_H
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
/*!\brief Base class for all declstrupp expression templates.
// \ingroup math
//
// The DeclStrUppExpr class serves as a tag for all expression templates that represent an
// explicit strictly upper declaration (declstrupp) operation. All classes, that represent a
// declstrupp operation and that are used within the expression template environment of the
// Metric numeric library have to derive publicly from this class in order to qualify as declstrupp
// expression template. Only in case a class is derived publicly from the DeclStrUppExpr
// base class, the IsDeclStrUppExpr type trait recognizes the class as valid declstrupp
// expression template.
*/
template <typename MT> // Matrix base type of the expression
struct DeclStrUppExpr : public DeclExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
