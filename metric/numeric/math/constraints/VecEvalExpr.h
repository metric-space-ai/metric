// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECEVALEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECEVALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVecEvalExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECEVALEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a vector evaluation expression (i.e. a type derived
// from the VecEvalExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECEVALEXPR_TYPE(T)                                                          \
	static_assert(::mtrc::numeric::IsVecEvalExpr_v<T>, "Non-vector evaluation expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECEVALEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a vector evaluation expression (i.e. a type derived from
// the VecEvalExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECEVALEXPR_TYPE(T)                                                      \
	static_assert(!::mtrc::numeric::IsVecEvalExpr_v<T>, "Vector evaluation expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
