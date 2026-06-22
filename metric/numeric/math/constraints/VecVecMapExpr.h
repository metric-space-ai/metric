// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECVECMAPEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECVECMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVecVecMapExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECVECMAPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a binary vector map expression (i.e. a type derived
// from the VecVecMapExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECVECMAPEXPR_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsVecVecMapExpr_v<T>, "Non-binary vector map expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECVECMAPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a binary vector map expression (i.e. a type derived from
// the VecVecMapExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECVECMAPEXPR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsVecVecMapExpr_v<T>, "Binary vector map expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
