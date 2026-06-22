// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_VOLATILE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_VOLATILE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsVolatile.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VOLATILE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is not a volatile-qualified type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VOLATILE(T)                                                                  \
	static_assert(::mtrc::numeric::IsVolatile_v<T>, "Non-volatile-qualified type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VOLATILE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is a volatile-qualified type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VOLATILE(T)                                                              \
	static_assert(!::mtrc::numeric::IsVolatile_v<T>, "Volatile-qualified type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
