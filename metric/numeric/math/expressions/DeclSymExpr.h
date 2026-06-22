// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DECLSYMEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DECLSYMEXPR_H
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
/*!\brief Base class for all declsym expression templates.
// \ingroup math
//
// The DeclSymExpr class serves as a tag for all expression templates that represent an
// explicit symmetry declaration (declsym) operation. All classes, that represent a declsym
// operation and that are used within the expression template environment of the Metric numeric library
// have to derive publicly from this class in order to qualify as declsym expression template.
// Only in case a class is derived publicly from the DeclSymExpr base class, the IsDeclSymExpr
// type trait recognizes the class as valid declsym expression template.
*/
template <typename MT> // Matrix base type of the expression
struct DeclSymExpr : public DeclExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
