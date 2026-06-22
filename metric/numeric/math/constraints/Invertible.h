// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_INVERTIBLE_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_INVERTIBLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsInvertible.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_INVERTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an invertible data type (i.e. a BLAS compatible type,
// <tt>long double</tt>, or any matrix with BLAS compatible element type), a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_INVERTIBLE_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsInvertible_v<T>, "Non-invertible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_INVERTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an invertible data type (i.e. a BLAS compatible type,
// <tt>long double</tt>, or any matrix with BLAS compatible element type), a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_INVERTIBLE_TYPE(T)                                                       \
	static_assert(!mtrc::numeric::IsInvertible_v<T>, "Invertible type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
