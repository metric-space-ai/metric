// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_EXPRESSION_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_EXPRESSION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsExpression.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_EXPRESSION_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an expression (i.e. a type derived from the Expression
// base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_EXPRESSION_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsExpression_v<T>, "Non-expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_EXPRESSION_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an expression (i.e. a type derived from the Expression
// base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_EXPRESSION_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsExpression_v<T>, "Expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
