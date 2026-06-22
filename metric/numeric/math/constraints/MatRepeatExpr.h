// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATREPEATEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATREPEATEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatRepeatExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATREPEATEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix repeater expression (i.e. a type derived from
// the MatRepeatExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATREPEATEXPR_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsMatRepeatExpr_v<T>, "Non-matrix repeater expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATREPEATEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix repeater expression (i.e. a type derived from the
// MatRepeatExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATREPEATEXPR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsMatRepeatExpr_v<T>, "Matrix repeater expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
