// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECTVECMULTEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECTVECMULTEXPR_H
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
/*!\brief Base class for all outer product expression templates.
// \ingroup math
//
// The VecTVecMultExpr class serves as a tag for all expression templates that implement
// mathematical outer products (i.e. multiplications between a column vector and a row vector).
// All classes, that represent a mathematical outer product and that are used within the
// expression template environment of the Metric numeric library have to derive publicly from this
// class in order to qualify as outer product expression template. Only in case a class is
// derived publicly from the VecTVecMultExpr base class, the IsVecTVecMultExpr type trait
// recognizes the class as valid outer product expression template.
*/
template <typename MT> // Matrix base type of the expression
struct VecTVecMultExpr : public MultExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
