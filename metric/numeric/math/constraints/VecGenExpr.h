// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECGENEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECGENEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVecGenExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECGENEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a vector generator expression (i.e. a type derived
// from the VecGenExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECGENEXPR_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsVecGenExpr_v<T>, "Non-vector generator expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECGENEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a vector generator expression (i.e. a type derived from
// the VecGenExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECGENEXPR_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsVecGenExpr_v<T>, "Vector generator expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
