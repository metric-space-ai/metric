// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATMATSOLVEEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATMATSOLVEEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/SolveExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all multi LSE solver expression templates.
// \ingroup math
//
// The MatMatSolveExpr class serves as a tag for all expression templates that implement a
// multi LSE solver operation (i.e. a solver expression for multiple right-hand side vectors).
// All classes, that represent a multi LSE solver operation and that are used within the
// expression template environment of the Metric numeric library have to derive publicly from this
// class in order to qualify as multi LSE solver expression template. Only in case a class
// is derived publicly from the MatMatSolveExpr base class, the IsMatMatSolveExpr type trait
// recognizes the class as valid multi LSE solver expression template.
*/
template <typename MT> // Matrix base type of the expression
struct MatMatSolveExpr : public SolveExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
