// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SUBEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SUBEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSubExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SUBEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a subtraction expression (i.e. a type derived from the
// SubExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBEXPR_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsSubExpr_v<T>, "Non-subtraction expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SUBEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a subtraction expression (i.e. a type derived from the
// SubExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SUBEXPR_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsSubExpr_v<T>, "Subtraction expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
