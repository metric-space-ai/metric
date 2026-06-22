// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_EXPANDEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_EXPANDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsExpandExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_EXPANDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an expansion expression (i.e. a type derived from the
// ExpandExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_EXPANDEXPR_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsExpandExpr_v<T>, "Non-expansion expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_EXPANDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an expansion expression (i.e. a type derived from the
// ExpandExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_EXPANDEXPR_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsExpandExpr_v<T>, "Expansion expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
