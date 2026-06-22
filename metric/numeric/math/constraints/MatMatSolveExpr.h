// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATMATSOLVEEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATMATSOLVEEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDenseMatrix.h>
#include <metric/numeric/math/typetraits/IsMatMatSolveExpr.h>
#include <metric/numeric/math/typetraits/Size.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATMATSOLVEEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a single LSE solver expression (i.e. a type derived
// from the MatMatSolveExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATMATSOLVEEXPR_TYPE(T)                                                      \
	static_assert(::mtrc::numeric::IsMatMatSolveExpr_v<T>, "Non-matrix/matrix solver expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATMATSOLVEEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a single LSE solver expression (i.e. a type derived from
// the MatMatSolveExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATMATSOLVEEXPR_TYPE(T)                                                  \
	static_assert(!::mtrc::numeric::IsMatMattSolveExpr_v<T>, "Matrix/matrix solver expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_FORM_VALID_MATMATSOLVEEXPR CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data types \a T1 and \a T2 do not form a valid single LSE solver, a
// compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_FORM_VALID_MATMATSOLVEEXPR(T1, T2)                                              \
	static_assert(::mtrc::numeric::IsDenseMatrix_v<T1> && ::mtrc::numeric::IsDenseMatrix_v<T2> &&                  \
					  (::mtrc::numeric::Size_v<T1, 0UL> == -1L || ::mtrc::numeric::Size_v<T1, 1UL> == -1L ||       \
					   ::mtrc::numeric::Size_v<T1, 0UL> == ::mtrc::numeric::Size_v<T1, 1UL>) &&                    \
					  ((::mtrc::numeric::Size_v<T1, 0UL> == -1L && ::mtrc::numeric::Size_v<T1, 1UL> == -1L) ||     \
					   (::mtrc::numeric::Size_v<T2, 0UL> == -1L) ||                                                  \
					   (::mtrc::numeric::Size_v<T1, 0UL> == ::mtrc::numeric::Size_v<T2, 0UL>) ||                   \
					   (::mtrc::numeric::Size_v<T1, 1UL> == ::mtrc::numeric::Size_v<T2, 0UL>)),                    \
				  "Invalid matrix/matrix solver expression detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
