// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_BLASCOMPATIBLE_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_BLASCOMPATIBLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsBLASCompatible.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_BLAS_COMPATIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a BLAS compatible data type (i.e. float, double,
// complex<float>, or complex<double>), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(T)                                                      \
	static_assert(::mtrc::numeric::IsBLASCompatible_v<T>, "Non-BLAS compatible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_BLAS_COMPATIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a BLAS compatible type (i.e. float, double, complex<float>,
// or complex<double>), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_BLAS_COMPATIBLE_TYPE(T)                                                  \
	static_assert(!::mtrc::numeric::IsBLASCompatible_v<T>, "BLAS compatible type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
