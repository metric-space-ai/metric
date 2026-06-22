// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATVECSOLVEEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATVECSOLVEEXPR_H
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
/*!\brief Base class for all single LSE solver expression templates.
// \ingroup math
//
// The MatVecSolveExpr class serves as a tag for all expression templates that implement a
// single LSE solver operation (i.e. a solver expression for a single right-hand side vector).
// All classes, that represent a single LSE solver operation and that are used within the
// expression template environment of the Metric numeric library have to derive publicly from this
// class in order to qualify as single LSE solver expression template. Only in case a class
// is derived publicly from the MatVecSolveExpr base class, the IsMatVecSolveExpr type trait
// recognizes the class as valid single LSE solver expression template.
*/
template <typename VT> // Vector base type of the expression
struct MatVecSolveExpr : public SolveExpr<VT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
