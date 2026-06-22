// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECVECKRONEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECVECKRONEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/KronExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all vector/vector Kronecker expression templates.
// \ingroup math
//
// The VecVecKronExpr class serves as a tag for all expression templates that implement a
// vector/vector Kronecker product. All classes, that represent a vector Kronecker product
// and that are used within the expression template environment of the Metric numeric library have to
// derive publicly from this class in order to qualify as vector Kronecker product expression
// template. Only in case a class is derived publicly from the VecVecKronExpr base class, the
// IsVecVecKronExpr type trait recognizes the class as valid vector Kronecker product expression
// template.
*/
template <typename VT> // Vector base type of the expression
struct VecVecKronExpr : public KronExpr<VT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
