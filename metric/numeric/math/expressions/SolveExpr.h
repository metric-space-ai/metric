// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SOLVEEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SOLVEEXPR_H
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
/*!\brief Base class for all LSE solver expression templates.
// \ingroup math
//
// The SolveExpr class serves as a tag for all expression templates that implement an LSE solver
// operation. All classes, that represent an LSE solver operation and that are used within the
// expression template environment of the Metric numeric library have to derive publicly from this class
// in order to qualify as LSE solver expression template. Only in case a class is derived publicly
// from the SolveExpr base class, the IsSolveExpr type trait recognizes the class as valid LSE
// solver expression template.
*/
template <typename T> // Base type of the expression
struct SolveExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
