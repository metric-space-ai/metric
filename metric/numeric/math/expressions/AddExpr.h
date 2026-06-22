// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_ADDEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_ADDEXPR_H
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
/*!\brief Base class for all addition expression templates.
// \ingroup math
//
// The AddExpr class serves as a tag for all expression templates that implement mathematical
// additions. All classes, that represent a mathematical addition (vector additions and matrix
// additions) and that are used within the expression template environment of the Metric numeric library
// have to derive publicly from this class in order to qualify as addition expression template.
// Only in case a class is derived publicly from the AddExpr base class, the IsAddExpr type
// trait recognizes the class as valid addition expression template.
*/
template <typename T> // Base type of the expression
struct AddExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
