// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATMATMULTEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATMATMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatMatMultExpr.h>
#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/Size.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATMATMULTEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix/matrix multiplication expression (i.e. a type
// derived from the MatMatMultExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATMATMULTEXPR_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsMatMatMultExpr_v<T>, "Non-matrix/matrix multiplication expression type "        \
															"detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATMATMULTEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix/matrix multiplication expression (i.e. a type
// derived from the MatMatMultExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATMATMULTEXPR_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsMatMatMultExpr_v<T>, "Matrix/matrix multiplication expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_FORM_VALID_MATMATMULTEXPR CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data types \a T1 and \a T2 do not form a valid matrix/matrix multiplication,
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_FORM_VALID_MATMATMULTEXPR(T1, T2)                                               \
	static_assert(::mtrc::numeric::IsMatrix_v<T1> && ::mtrc::numeric::IsMatrix_v<T2> &&                            \
					  ((::mtrc::numeric::Size_v<T1, 1UL> == -1L) || (::mtrc::numeric::Size_v<T2, 0UL> == -1L) ||   \
					   (::mtrc::numeric::Size_v<T1, 1UL> == ::mtrc::numeric::Size_v<T2, 0UL>)),                    \
				  "Invalid matrix/matrix multiplication expression detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
