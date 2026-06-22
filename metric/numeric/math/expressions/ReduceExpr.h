// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_REDUCEEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_REDUCEEXPR_H
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
/*!\brief Base class for all reduction expression templates.
// \ingroup math
//
// The ReduceExpr class serves as a tag for all expression templates that implement reduction
// operations. All classes, that represent a reduction operation (e.g. row-wise or column-wise
// matrix reductions) and that are used within the expression template environment of the Metric numeric
// library have to derive publicly from this class in order to qualify as reduction expression
// template. Only in case a class is derived publicly from the ReduceExpr base class, the
// IsReduceExpr type trait recognizes the class as valid reduction expression template.
*/
template <typename T> // Base type of the expression
struct ReduceExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
