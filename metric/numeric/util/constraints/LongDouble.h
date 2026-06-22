// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_LONGDOUBLE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_LONGDOUBLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsLongDouble.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_LONGDOUBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is of type long double
// (ignoring the cv-qualifiers). In case \a T is not of type long double, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_LONGDOUBLE_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsLongDouble_v<T>, "Non-long double type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_LONGDOUBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is not of type long double
// (ignoring the cv-qualifiers). In case \a T is of type long double, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_LONGDOUBLE_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsLongDouble_v<T>, "Long double type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
