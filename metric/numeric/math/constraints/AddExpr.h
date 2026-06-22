// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_ADDEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_ADDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsAddExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_ADDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an addition expression (i.e. a type derived from the
// AddExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_ADDEXPR_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsAddExpr_v<T>, "Non-addition expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_ADDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an addition expression (i.e. a type derived from the
// AddExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADDEXPR_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsAddExpr_v<T>, "Addition expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
