// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATREDUCEEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATREDUCEEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatReduceExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATREDUCEEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix reduction expression (i.e. a type derived
// from the MatReduceExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATREDUCEEXPR_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsMatReduceExpr_v<T>, "Non-matrix reduction expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATREDUCEEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix reduction expression (i.e. a type derived from
// the MatReduceExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATREDUCEEXPR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsMatReduceExpr_v<T>, "Matrix reduction expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
