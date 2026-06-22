// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATMATSUBEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATMATSUBEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatMatSubExpr.h>
#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/Size.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATMATSUBEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix/matrix subtraction expression (i.e. a type
// derived from the MatMatSubExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATMATSUBEXPR_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsMatMatSubExpr_v<T>, "Non-matrix/matrix subtraction expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATMATSUBEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix/matrix subtraction expression (i.e. a type derived
// from the MatMatSubExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATMATSUBEXPR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsMatMatSubExpr_v<T>, "Matrix/matrix subtraction expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_FORM_VALID_MATMATSUBEXPR CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data types \a T1 and \a T2 do not form a valid matrix/matrix subtraction,
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_FORM_VALID_MATMATSUBEXPR(T1, T2)                                                \
	static_assert(::mtrc::numeric::IsMatrix_v<T1> && ::mtrc::numeric::IsMatrix_v<T2> &&                            \
					  ((::mtrc::numeric::Size_v<T1, 0UL> == -1L) || (::mtrc::numeric::Size_v<T2, 0UL> == -1L) ||   \
					   (::mtrc::numeric::Size_v<T1, 0UL> == ::mtrc::numeric::Size_v<T2, 0UL>)) &&                  \
					  ((::mtrc::numeric::Size_v<T1, 1UL> == -1L) || (::mtrc::numeric::Size_v<T2, 1UL> == -1L) ||   \
					   (::mtrc::numeric::Size_v<T1, 1UL> == ::mtrc::numeric::Size_v<T2, 1UL>)),                    \
				  "Invalid matrix/matrix subtraction expression detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
