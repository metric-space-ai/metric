// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECVECSUBEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECVECSUBEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/SubExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all vector/vector subtraction expression templates.
// \ingroup math
//
// The VecVecSubExpr class serves as a tag for all expression templates that implement a
// vector/vector subtraction. All classes, that represent a vector subtraction and that
// are used within the expression template environment of the Metric numeric library have to derive
// publicly from this class in order to qualify as vector subtraction expression template. Only
// in case a class is derived publicly from the VecVecSubExpr base class, the IsVecVecSubExpr
// type trait recognizes the class as valid vector subtraction expression template.
*/
template <typename VT> // Vector base type of the expression
struct VecVecSubExpr : public SubExpr<VT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
