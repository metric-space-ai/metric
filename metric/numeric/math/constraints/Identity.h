// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_IDENTITY_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_IDENTITY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsIdentity.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_IDENTITY_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an identity matrix type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_IDENTITY_MATRIX_TYPE(T)                                                      \
	static_assert(::mtrc::numeric::IsIdentity_v<T>, "Non-identity matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_IDENTITY_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an identity matrix type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_IDENTITY_MATRIX_TYPE(T)                                                  \
	static_assert(!::mtrc::numeric::IsIdentity_v<T>, "Identity matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
