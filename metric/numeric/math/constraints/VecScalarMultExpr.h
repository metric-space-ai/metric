// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECSCALARMULTEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECSCALARMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVecScalarMultExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECSCALARMULTEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a vector/scalar multiplication expression (i.e. a type
// derived from the VecScalarMultExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECSCALARMULTEXPR_TYPE(T)                                                    \
	static_assert(::mtrc::numeric::IsVecScalarMultExpr_v<T>,                                                         \
				  "Non-vector/scalar multiplication expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECSCALARMULTEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a vector/scalar multiplication expression (i.e. a type
// derived from the VecScalarMultExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECSCALARMULTEXPR_TYPE(T)                                                \
	static_assert(!::mtrc::numeric::IsVecScalarMultExpr_v<T>, "Vector/scalar multiplication expression type "        \
																"detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
