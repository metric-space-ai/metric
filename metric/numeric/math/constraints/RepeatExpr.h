// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_REPEATEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_REPEATEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsRepeatExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_REPEATEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a repeater expression (i.e. a type derived from the
// RepeatExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_REPEATEXPR_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsRepeatExpr_v<T>, "Non-repeater expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_REPEATEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a repeater expression (i.e. a type derived from the
// RepeatExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_REPEATEXPR_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsRepeatExpr_v<T>, "Repeater expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
