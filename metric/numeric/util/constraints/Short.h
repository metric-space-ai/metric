// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_SHORT_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_SHORT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsShort.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SHORT_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is a \c short data type
// (ignoring the cv-qualifiers). In case \a T is not a \c short data type, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SHORT_TYPE(T)                                                                \
	static_assert(::mtrc::numeric::IsShort_v<T>, "Non-short type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SHORT_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is not a \c short data type
// (ignoring the cv-qualifiers). In case \a T is a \c short data type, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SHORT_TYPE(T)                                                            \
	static_assert(!::mtrc::numeric::IsShort_v<T>, "Short type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
