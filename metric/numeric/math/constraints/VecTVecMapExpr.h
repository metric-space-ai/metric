// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECTVECMAPEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECTVECMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVecTVecMapExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECTVECMAPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an outer map expression (i.e. a type derived from the
// VecTVecMapExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTVECMAPEXPR_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsVecTVecMapExpr_v<T>, "Non-outer map expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECTVECMAPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an outer map expression (i.e. a type derived from the
// VecTVecMapExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECTVECMAPEXPR_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsVecTVecMapExpr_v<T>, "Outer map expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
