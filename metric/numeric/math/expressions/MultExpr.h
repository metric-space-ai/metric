// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MULTEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MULTEXPR_H
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
/*!\brief Base class for all multiplication expression templates.
// \ingroup math
//
// The MultExpr class serves as a tag for all expression templates that implement mathematical
// multiplications. All classes, that represent a mathematical multiplication (element-wise
// vector multiplications, matrix/vector multiplications, vector/matrix multiplications and
// matrix/matrix multiplications) and that are used within the expression template environment
// of the Metric numeric library have to derive publicly from this class in order to qualify as
// multiplication expression template. Only in case a class is derived publicly from the
// MultExpr base class, the IsMultExpr type trait recognizes the class as valid multiplication
// expression template.
*/
template <typename T> // Base type of the expression
struct MultExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
