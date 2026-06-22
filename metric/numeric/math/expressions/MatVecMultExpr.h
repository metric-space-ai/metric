// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATVECMULTEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATVECMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/MultExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all matrix/vector multiplication expression templates.
// \ingroup math
//
// The MatVecMultExpr class serves as a tag for all expression templates that implement a
// matrix/vector multiplication. All classes, that represent a matrix/vector multiplication
// and that are used within the expression template environment of the Metric numeric library have
// to derive publicly from this class in order to qualify as matrix/vector multiplication
// expression template. Only in case a class is derived publicly from the MatVecMultExpr
// base class, the IsMatVecMultExpr type trait recognizes the class as valid matrix/vector
// multiplication expression template.
*/
template <typename VT> // Vector base type of the expression
struct MatVecMultExpr : public MultExpr<VT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
