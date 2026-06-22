// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECTRANSEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECTRANSEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVecTransExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECTRANSEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a vector transposition expression (i.e. a type derived
// from the VecTransExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTRANSEXPR_TYPE(T)                                                         \
	static_assert(::mtrc::numeric::IsVecTransExpr_v<T>, "Non-vector transposition expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECTRANSEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a vector transposition expression (i.e. a type derived from
// the VecTransExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECTRANSEXPR_TYPE(T)                                                     \
	static_assert(!::mtrc::numeric::IsVecTransExpr_v<T>, "Vector transposition expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
