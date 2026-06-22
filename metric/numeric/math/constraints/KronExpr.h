// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_KRONEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_KRONEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsKronExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_KRONEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a Kronecker product expression (i.e. a type derived
// from the KronExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_KRONEXPR_TYPE(T)                                                             \
	static_assert(::mtrc::numeric::IsKronExpr_v<T>, "Non-Kronecker product expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_KRONEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a Kronecker product expression (i.e. a type derived from
// the KronExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_KRONEXPR_TYPE(T)                                                         \
	static_assert(!::mtrc::numeric::IsKronExpr_v<T>, "Kronecker product expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
