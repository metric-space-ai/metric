// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATTRANSEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATTRANSEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatTransExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATTRANSEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix transposition expression (i.e. a type derived
// from the MatTransExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATTRANSEXPR_TYPE(T)                                                         \
	static_assert(::mtrc::numeric::IsMatTransExpr_v<T>, "Non-matrix transposition expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATTRANSEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix transposition expression (i.e. a type derived from
// the MatTransExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATTRANSEXPR_TYPE(T)                                                     \
	static_assert(!::mtrc::numeric::IsMatTransExpr_v<T>, "Matrix transposition expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
