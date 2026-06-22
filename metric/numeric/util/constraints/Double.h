// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_DOUBLE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_DOUBLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsDouble.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DOUBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is of type double
// (ignoring the cv-qualifiers). In case \a T is not of type double, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DOUBLE_TYPE(T)                                                               \
	static_assert(::mtrc::numeric::IsDouble_v<T>, "Non-double type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DOUBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is not of type double
// (ignoring the cv-qualifiers). In case \a T is of type double, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DOUBLE_TYPE(T)                                                           \
	static_assert(!::mtrc::numeric::IsDouble_v<T>, "Double type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
