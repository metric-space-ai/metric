// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATINVEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATINVEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatInvExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATINVEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix inversion expression (i.e. a type derived
// from the MatInvExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATINVEXPR_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsMatInvExpr_v<T>, "Non-matrix inversion expression detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATINVEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix inversion expression (i.e. a type derived from
// the MatInvExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATINVEXPR_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsMatInvExpr_v<T>, "Matrix inversion expression detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
