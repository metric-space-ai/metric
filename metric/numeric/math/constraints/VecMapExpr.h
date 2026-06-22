// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECMAPEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVecMapExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECMAPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a unary vector map expression (i.e. a type derived
// from the VecMapExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECMAPEXPR_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsVecMapExpr_v<T>, "Non-unary vector map expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECMAPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a unary vector map expression (i.e. a type derived from
// the VecMapExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECMAPEXPR_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsVecMapExpr_v<T>, "Unary vector map expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
