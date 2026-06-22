// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECTVECMULTEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECTVECMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsColumnVector.h>
#include <metric/numeric/math/typetraits/IsRowVector.h>
#include <metric/numeric/math/typetraits/IsVecTVecMultExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECTVECMULTEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an outer product expression (i.e. a type derived from
// the VecTVecMultExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTVECMULTEXPR_TYPE(T)                                                      \
	static_assert(::mtrc::numeric::IsVecTVecMultExpr_v<T>, "Non-outer product expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECTVECMULTEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an outer product expression (i.e. a type derived from the
// VecTVecMultExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECTVECMULTEXPR_TYPE(T)                                                  \
	static_assert(!::mtrc::numeric::IsVecTVecMultExpr_v<T>, "Outer product expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_FORM_VALID_VECTVECMULTEXPR CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data types \a T1 and \a T2 do not form a valid vector/vector multiplication,
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_FORM_VALID_VECTVECMULTEXPR(T1, T2)                                              \
	static_assert(::mtrc::numeric::IsColumnVector_v<T1> && ::mtrc::numeric::IsRowVector_v<T2>,                     \
				  "Invalid vector/vector multiplication expression detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
