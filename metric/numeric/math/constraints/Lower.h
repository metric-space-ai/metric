// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_LOWER_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_LOWER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsLower.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_LOWER_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a lower triangular matrix type, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_LOWER_MATRIX_TYPE(T)                                                         \
	static_assert(::mtrc::numeric::IsLower_v<T>, "Non-lower triangular matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_LOWER_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a lower triangular matrix type, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_LOWER_MATRIX_TYPE(T)                                                     \
	static_assert(!::mtrc::numeric::IsLower_v<T>, "Lower triangular matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
