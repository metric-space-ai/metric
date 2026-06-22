// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATGENEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATGENEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatGenExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATGENEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix generator expression (i.e. a type derived
// from the MatGenExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATGENEXPR_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsMatGenExpr_v<T>, "Non-matrix generator expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATGENEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix generator expression (i.e. a type derived from
// the MatGenExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATGENEXPR_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsMatGenExpr_v<T>, "Matrix generator expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
