// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_TVECMATMULTEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_TVECMATMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsRowVector.h>
#include <metric/numeric/math/typetraits/IsTVecMatMultExpr.h>
#include <metric/numeric/math/typetraits/Size.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_TVECMATMULTEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a vector/matrix multiplication expression (i.e. a type
// derived from the TVecMatMultExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_TVECMATMULTEXPR_TYPE(T)                                                      \
	static_assert(::mtrc::numeric::IsTVecMatMultExpr_v<T>,                                                           \
				  "Non-vector/matrix multiplication expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_TVECMATMULTEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a vector/matrix multiplication expression (i.e. a type
// derived from the TVecMatMultExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_TVECMATMULTEXPR_TYPE(T)                                                  \
	static_assert(!::mtrc::numeric::IsTVecMatMultExpr_v<T>, "Vector/matrix multiplication expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_FORM_VALID_TVECMATMULTEXPR CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data types \a T1 and \a T2 do not form a valid vector/matrix multiplication,
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_FORM_VALID_TVECMATMULTEXPR(T1, T2)                                              \
	static_assert(::mtrc::numeric::IsRowVector_v<T1> && ::mtrc::numeric::IsMatrix_v<T2> &&                         \
					  ((::mtrc::numeric::Size_v<T1, 0UL> == -1L) || (::mtrc::numeric::Size_v<T2, 0UL> == -1L) ||   \
					   (::mtrc::numeric::Size_v<T1, 0UL> == ::mtrc::numeric::Size_v<T2, 0UL>)),                    \
				  "Invalid vector/matrix multiplication expression detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
