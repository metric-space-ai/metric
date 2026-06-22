// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATMAPEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/UnaryMapExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all unary matrix map expression templates.
// \ingroup math
//
// The MatMapExpr class serves as a tag for all expression templates that represent a unary map
// operation on a matrix. All classes, that represent a unary matrix map operation and that are
// used within the expression template environment of the Metric numeric library have to derive publicly
// from this class in order to qualify as unary matrix map expression template. Only in case a
// class is derived publicly from the MatMapExpr base class, the IsMatMapExpr type trait
// recognizes the class as valid unary matrix map expression template.
*/
template <typename MT> // Matrix base type of the expression
struct MatMapExpr : public UnaryMapExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
