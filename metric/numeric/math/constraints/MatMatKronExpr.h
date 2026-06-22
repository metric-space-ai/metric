// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATMATKRONEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATMATKRONEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatMatKronExpr.h>
#include <metric/numeric/math/typetraits/IsMatrix.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATMATKRONEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix/matrix Kronecker product expression (i.e. a
// type derived from the MatMatKronExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATMATKRONEXPR_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsMatMatKronExpr_v<T>,                                                            \
				  "Non-matrix/matrix Kronecker product expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATMATKRONEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix/matrix Kronecker product expression (i.e. a type
// derived from the MatMatKronExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATMATKRONEXPR_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsMatMatKronExpr_v<T>, "Matrix/matrix Kronecker product expression type "        \
															 "detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_FORM_VALID_MATMATKRONEXPR CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data types \a T1 and \a T2 do not form a valid matrix/matrix Kronecker
// product, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_FORM_VALID_MATMATKRONEXPR(T1, T2)                                               \
	static_assert(::mtrc::numeric::IsMatrix_v<T1> && ::mtrc::numeric::IsMatrix_v<T2>,                              \
				  "Invalid matrix/matrix Kronecker product expression detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
