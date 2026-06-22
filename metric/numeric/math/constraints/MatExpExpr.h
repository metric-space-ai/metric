// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATEXPEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATEXPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatExpExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATEXPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix exponential expression (i.e. a type derived
// from the MatExpExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATEXPEXPR_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsMatExpExpr_v<T>, "Non-matrix exponential expression detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATEXPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix exponential expression (i.e. a type derived from
// the MatExpExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATEXPEXPR_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsMatExpExpr_v<T>, "Matrix exponential expression detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
