// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECVECMULTEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECVECMULTEXPR_H
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
/*!\brief Base class for all vector/vector multiplication expression templates.
// \ingroup math
//
// The VecVecMultExpr class serves as a tag for all expression templates that implement a
// vector/vector multiplication. All classes, that represent a vector multiplication and that are
// used within the expression template environment of the Metric numeric library have to derive publicly
// from this class in order to qualify as vector multiplication expression template. Only in case
// a class is derived publicly from the VecVecMultExpr base class, the IsVecVecMultExpr type trait
// recognizes the class as valid vector multiplication expression template.
*/
template <typename VT> // Vector base type of the expression
struct VecVecMultExpr : public MultExpr<VT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
