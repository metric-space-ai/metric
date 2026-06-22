// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECVECDIVEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECVECDIVEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/DivExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all vector/vector division expression templates.
// \ingroup math
//
// The VecVecDivExpr class serves as a tag for all expression templates that implement a
// vector/vector division. All classes, that represent a vector division and that are used
// within the expression template environment of the Metric numeric library have to derive publicly
// from this class in order to qualify as vector division expression template. Only in case
// a class is derived publicly from the VecVecDivExpr base class, the IsVecVecDivExpr type
// trait recognizes the class as valid vector division expression template.
*/
template <typename VT> // Vector base type of the expression
struct VecVecDivExpr : public DivExpr<VT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
