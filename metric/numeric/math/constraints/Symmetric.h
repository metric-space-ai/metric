// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SYMMETRIC_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SYMMETRIC_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSymmetric.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SYMMETRIC_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a symmetric matrix type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SYMMETRIC_MATRIX_TYPE(T)                                                     \
	static_assert(::mtrc::numeric::IsSymmetric_v<T>, "Non-symmetric matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a symmetric matrix type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(T)                                                 \
	static_assert(!::mtrc::numeric::IsSymmetric_v<T>, "Symmetric matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
