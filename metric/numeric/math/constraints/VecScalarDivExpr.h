// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECSCALARDIVEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECSCALARDIVEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVecScalarDivExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECSCALARDIVEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a vector/scalar division expression (i.e. a type
// derived from the VecScalarDivExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECSCALARDIVEXPR_TYPE(T)                                                     \
	static_assert(::mtrc::numeric::IsVecScalarDivExpr_v<T>, "Non-vector/scalar division expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECSCALARDIVEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a vector/scalar division expression (i.e. a type derived
// from the VecScalarDivExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECSCALARDIVEXPR_TYPE(T)                                                 \
	static_assert(!::mtrc::numeric::IsVecScalarDivExpr_v<T>, "Vector/scalar division expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
