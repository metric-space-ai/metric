// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_INTEGER_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_INTEGER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsInteger.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_INTEGER_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is of type \c int (ignoring
// the cv-qualifiers). In case \a T is not of type \c int, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_INTEGER_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsInteger_v<T>, "Non-integer type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_INTEGER_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is not of type \c int
// (ignoring the cv-qualifiers). In case \a T is of type \c int, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_INTEGER_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsInteger_v<T>, "Integer type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
