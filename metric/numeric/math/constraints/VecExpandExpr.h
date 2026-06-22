// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECEXPANDEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECEXPANDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVecExpandExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECEXPANDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a vector expansion expression (i.e. a type derived
// from the VecExpandExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECEXPANDEXPR_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsVecExpandExpr_v<T>, "Non-vector expansion expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECEXPANDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a vector expansion expression (i.e. a type derived from
// the VecExpandExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECEXPANDEXPR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsVecExpandExpr_v<T>, "Vector expansion expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
