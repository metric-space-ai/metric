// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECVECSUBEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECVECSUBEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsColumnVector.h>
#include <metric/numeric/math/typetraits/IsRowVector.h>
#include <metric/numeric/math/typetraits/IsVecVecSubExpr.h>
#include <metric/numeric/math/typetraits/Size.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECVECSUBEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a vector/vector subtraction expression (i.e. a type
// derived from the VecVecSubExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECVECSUBEXPR_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsVecVecSubExpr_v<T>, "Non-vector/vector subtraction expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECVECSUBEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a vector/vector subtraction expression (i.e. a type derived
// from the VecVecSubExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECVECSUBEXPR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsVecVecSubExpr_v<T>, "Vector/vector subtraction expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_FORM_VALID_VECVECSUBEXPR CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data types \a T1 and \a T2 do not form a valid vector/vector subtraction,
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_FORM_VALID_VECVECSUBEXPR(T1, T2)                                                \
	static_assert(((::mtrc::numeric::IsRowVector_v<T1> && ::mtrc::numeric::IsRowVector_v<T2>) ||                   \
				   (::mtrc::numeric::IsColumnVector_v<T1> && ::mtrc::numeric::IsColumnVector_v<T2>)) &&            \
					  ((::mtrc::numeric::Size_v<T1, 0UL> == -1L) || (::mtrc::numeric::Size_v<T2, 0UL> == -1L) ||   \
					   (::mtrc::numeric::Size_v<T1, 0UL> == ::mtrc::numeric::Size_v<T2, 0UL>)),                    \
				  "Invalid vector/vector subtraction expression detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
