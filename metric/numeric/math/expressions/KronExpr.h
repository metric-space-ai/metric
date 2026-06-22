// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_KRONEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_KRONEXPR_H
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
/*!\brief Base class for all Kron product expression templates.
// \ingroup math
//
// The KronExpr class serves as a tag for all expression templates that implement mathematical
// Kronecker products. All classes, that represent a mathematical addition (Kronecker products
// between vectors or matrices) and that are used within the expression template environment of
// the Metric numeric library have to derive publicly from this class in order to qualify as Kronecker
// product expression template. Only in case a class is derived publicly from the KronExpr base
// class, the IsKronExpr type trait recognizes the class as valid Kronecker product expression
// template.
*/
template <typename MT> // Matrix base type of the expression
struct KronExpr : public Expression<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
