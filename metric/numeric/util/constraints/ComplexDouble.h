// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_COMPLEXDOUBLE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_COMPLEXDOUBLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsComplexDouble.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_COMPLEX_DOUBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is a double precision
// complex data type (i.e. \a complex<double>, ignoring the cv-qualifiers). In case \a T is
// not of type \a complex<double> a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_COMPLEX_DOUBLE_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsComplexDouble_v<T>, "Non-double precision complex type detected")
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
// This compile time constraint checks that the given data type \a T is not a double precision
// complex data type (i.e. \a complex<double>, ignoring the cv-qualifiers). In case \a T is of
// type \a complex<double> a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPLEX_DOUBLE_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsComplexDouble_v<T>, "Double precision complex type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
