// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_TVECMATMULTEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_TVECMATMULTEXPR_H
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
/*!\brief Base class for all vector/matrix multiplication expression templates.
// \ingroup math
//
// The TVecMatMultExpr class serves as a tag for all expression templates that implement a
// vector/matrix multiplication. All classes, that represent a vector/matrix multiplication
// and that are used within the expression template environment of the Metric numeric library have
// to derive publicly from this class in order to qualify as vector/matrix multiplication
// expression template. Only in case a class is derived publicly from the TVecMatMultExpr
// base class, the IsTVecMatMultExpr type trait recognizes the class as valid vector/matrix
// multiplication expression template.
*/
template <typename VT> // Vector base type of the expression
struct TVecMatMultExpr : public MultExpr<VT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
