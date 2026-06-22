// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_UPPER_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_UPPER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsUpper.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_UPPER_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a upper triangular matrix type, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_UPPER_MATRIX_TYPE(T)                                                         \
	static_assert(::mtrc::numeric::IsUpper_v<T>, "Non-upper triangular matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_UPPER_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a upper triangular matrix type, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UPPER_MATRIX_TYPE(T)                                                     \
	static_assert(!::mtrc::numeric::IsUpper_v<T>, "Upper triangular matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
