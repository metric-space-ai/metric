// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATVECMULTEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATVECMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsColumnVector.h>
#include <metric/numeric/math/typetraits/IsMatVecMultExpr.h>
#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/Size.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATVECMULTEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix/vector multiplication expression (i.e. a type
// derived from the MatVecMultExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATVECMULTEXPR_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsMatVecMultExpr_v<T>, "Non-matrix/vector multiplication expression type "        \
															"detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATVECMULTEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix/vector multiplication expression (i.e. a type
// derived from the MatVecMultExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATVECMULTEXPR_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsMatVecMultExpr_v<T>, "Matrix/vector multiplication expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_FORM_VALID_MATVECMULTEXPR CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data types \a T1 and \a T2 do not form a valid matrix/vector multiplication,
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_FORM_VALID_MATVECMULTEXPR(T1, T2)                                               \
	static_assert(::mtrc::numeric::IsMatrix_v<T1> && ::mtrc::numeric::IsColumnVector_v<T2> &&                      \
					  ((::mtrc::numeric::Size_v<T1, 1UL> == -1L) || (::mtrc::numeric::Size_v<T2, 0UL> == -1L) ||   \
					   (::mtrc::numeric::Size_v<T1, 1UL> == ::mtrc::numeric::Size_v<T2, 0UL>)),                    \
				  "Invalid matrix/vector multiplication expression detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
