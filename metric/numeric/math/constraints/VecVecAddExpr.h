// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECVECADDEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECVECADDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsColumnVector.h>
#include <metric/numeric/math/typetraits/IsRowVector.h>
#include <metric/numeric/math/typetraits/IsVecVecAddExpr.h>
#include <metric/numeric/math/typetraits/Size.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECVECADDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a vector/vector addition expression (i.e. a type
// derived from the VecVecAddExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECVECADDEXPR_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsVecVecAddExpr_v<T>, "Non-vector/vector addition expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECVECADDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a vector/vector addition expression (i.e. a type derived
// from the VecVecAddExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECVECADDEXPR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsVecVecAddExpr_v<T>, "Vector/vector addition expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_FORM_VALID_VECVECADDEXPR CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data types \a T1 and \a T2 do not form a valid vector/vector addition,
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_FORM_VALID_VECVECADDEXPR(T1, T2)                                                \
	static_assert(((::mtrc::numeric::IsRowVector_v<T1> && ::mtrc::numeric::IsRowVector_v<T2>) ||                   \
				   (::mtrc::numeric::IsColumnVector_v<T1> && ::mtrc::numeric::IsColumnVector_v<T2>)) &&            \
					  ((::mtrc::numeric::Size_v<T1, 0UL> == -1L) || (::mtrc::numeric::Size_v<T2, 0UL> == -1L) ||   \
					   (::mtrc::numeric::Size_v<T1, 0UL> == ::mtrc::numeric::Size_v<T2, 0UL>)),                    \
				  "Invalid vector/vector addition expression detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
