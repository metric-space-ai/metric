// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_UNIUPPER_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_UNIUPPER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsUniUpper.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_UNIUPPER_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a upper unitriangular matrix type, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_UNIUPPER_MATRIX_TYPE(T)                                                      \
	static_assert(::mtrc::numeric::IsUniUpper_v<T>, "Non-upper unitriangular matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_UNIUPPER_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a upper unitriangular matrix type, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UNIUPPER_MATRIX_TYPE(T)                                                  \
	static_assert(!::mtrc::numeric::IsUniUpper_v<T>, "Upper unitriangular matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
