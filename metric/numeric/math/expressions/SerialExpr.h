// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SERIALEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SERIALEXPR_H
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
/*!\brief Base class for all serial evaluation expression templates.
// \ingroup math
//
// The SerialExpr class serves as a tag for all expression templates that enforce a serial
// evaluation. All classes, that represent a serialization operation and that are used within
// the expression template environment of the Metric numeric library have to derive publicly from this
// class in order to qualify as serial evaluation expression template. Only in case a class is
// derived publicly from the SerialExpr base class, the IsSerialExpr type trait recognizes the
// class as valid serial evaluation expression template.
*/
template <typename T> // Base type of the expression
struct SerialExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
