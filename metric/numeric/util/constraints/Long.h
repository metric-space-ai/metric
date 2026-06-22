// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_LONG_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_LONG_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsLong.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_LONG_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is a \c long data type
// (ignoring the cv-qualifiers). In case \a T is not a \c long data type, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_LONG_TYPE(T)                                                                 \
	static_assert(::mtrc::numeric::IsLong_v<T>, "Non-long type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_LONG_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is not a \c long data type
// (ignoring the cv-qualifiers). In case \a T is a \c long data type, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_LONG_TYPE(T)                                                             \
	static_assert(!::mtrc::numeric::IsLong_v<T>, "Long type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
