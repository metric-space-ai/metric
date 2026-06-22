// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATMATKRONEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATMATKRONEXPR_H
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
/*!\brief Base class for all matrix/matrix Kronecker expression templates.
// \ingroup math
//
// The MatMatKronExpr class serves as a tag for all expression templates that implement a
// matrix/matrix Kronecker product. All classes, that represent a matrix Kronecker product
// and that are used within the expression template environment of the Metric numeric library have to
// derive publicly from this class in order to qualify as matrix Kronecker product expression
// template. Only in case a class is derived publicly from the MatMatKronExpr base class, the
// IsMatMatKronExpr type trait recognizes the class as valid matrix Kronecker product expression
// template.
*/
template <typename MT> // Matrix base type of the expression
struct MatMatKronExpr : public KronExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
