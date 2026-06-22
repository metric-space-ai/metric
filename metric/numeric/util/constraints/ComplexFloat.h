// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_COMPLEXFLOAT_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_COMPLEXFLOAT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsComplexFloat.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_COMPLEX_FLOAT_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is a single precision
// complex data type (i.e. \a complex<float>, ignoring the cv-qualifiers). In case \a T is
// not of type \a complex<float> a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_COMPLEX_FLOAT_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsComplexFloat_v<T>, "Non-single precision complex type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_COMPLEX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is not a single precision
// complex data type (i.e. \a complex<float>, ignoring the cv-qualifiers). In case \a T is of
// type \a complex<float> a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPLEX_FLOAT_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsComplexFloat_v<T>, "Single precision complex type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
