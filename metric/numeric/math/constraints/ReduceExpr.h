// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_REDUCEEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_REDUCEEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsReduceExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_REDUCEEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a reduce expression (i.e. a type derived from the
// ReduceExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_REDUCEEXPR_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsReduceExpr_v<T>, "Non-reduce expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_REDUCEEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a reduce expression (i.e. a type derived from the
// ReduceExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_REDUCEEXPR_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsReduceExpr_v<T>, "Reduce expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
