// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DECLDIAGEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DECLDIAGEXPR_H
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
/*!\brief Base class for all decldiag expression templates.
// \ingroup math
//
// The DeclDiagExpr class serves as a tag for all expression templates that represent an
// explicit diagonal declaration (decldiag) operation. All classes, that represent a decldiag
// operation and that are used within the expression template environment of the Metric numeric library
// have to derive publicly from this class in order to qualify as decldiag expression template.
// Only in case a class is derived publicly from the DeclDiagExpr base class, the IsDeclDiagExpr
// type trait recognizes the class as valid decldiag expression template.
*/
template <typename MT> // Matrix base type of the expression
struct DeclDiagExpr : public DeclExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
