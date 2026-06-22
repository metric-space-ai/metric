// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SCHUREXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SCHUREXPR_H
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
/*!\brief Base class for all Schur product expression templates.
// \ingroup math
//
// The SchurExpr class serves as a tag for all expression templates that implement mathematical
// Schur products. All classes, that represent a mathematical Schur product and that are used
// within the expression template environment of the Metric numeric library have to derive publicly from
// this class in order to qualify as Schur product expression template. Only in case a class is
// derived publicly from the SchurExpr base class, the IsSchurExpr type trait recognizes the
// class as valid Schur product expression template.
*/
template <typename MT> // Matrix base type of the expression
struct SchurExpr : public Expression<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
