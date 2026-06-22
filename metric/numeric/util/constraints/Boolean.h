// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_BOOLEAN_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_BOOLEAN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsBoolean.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_BOOLEAN_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not a boolean, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_BOOLEAN_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsBoolean_v<T>, "Non-boolean type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_BOOLEAN_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is a boolean, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_BOOLEAN_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsBoolean_v<T>, "Boolean type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
