// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_HERMITIAN_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_HERMITIAN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsHermitian.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_HERMITIAN_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an Hermitian matrix type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_HERMITIAN_MATRIX_TYPE(T)                                                     \
	static_assert(::mtrc::numeric::IsHermitian_v<T>, "Non-Hermitian matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_HERMITIAN_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an Hermitian matrix type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_HERMITIAN_MATRIX_TYPE(T)                                                 \
	static_assert(!::mtrc::numeric::IsHermitian_v<T>, "Hermitian matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
