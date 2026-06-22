// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECVECKRONEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECVECKRONEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVecVecKronExpr.h>
#include <metric/numeric/math/typetraits/IsVector.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECVECKRONEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a vector/vector Kronecker product expression (i.e. a
// type derived from the VecVecKronExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECVECKRONEXPR_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsVecVecKronExpr_v<T>,                                                            \
				  "Non-vector/vector Kronecker product expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECVECKRONEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a vector/vector Kronecker product expression (i.e. a type
// derived from the VecVecKronExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECVECKRONEXPR_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsVecVecKronExpr_v<T>, "Vector/vector Kronecker product expression type "        \
															 "detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_FORM_VALID_VECVECKRONEXPR CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data types \a T1 and \a T2 do not form a valid vector/vector Kronecker
// product, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_FORM_VALID_VECVECKRONEXPR(T1, T2)                                               \
	static_assert(::mtrc::numeric::IsVector_v<T1> && ::mtrc::numeric::IsVector_v<T2>,                              \
				  "Invalid vector/vector Kronecker product expression detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
