// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_STRICTLYUPPER_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_STRICTLYUPPER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsStrictlyUpper.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_STRICTLY_UPPER_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a strictly upper triangular matrix type, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_STRICTLY_UPPER_MATRIX_TYPE(T)                                                \
	static_assert(::mtrc::numeric::IsStrictlyUpper_v<T>, "Non-strictly upper triangular matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_STRICTLY_UPPER_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a strictly upper triangular matrix type, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_UPPER_MATRIX_TYPE(T)                                            \
	static_assert(!::mtrc::numeric::IsStrictlyUpper_v<T>, "Strictly upper triangular matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
