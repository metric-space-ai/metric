// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_BUILTIN_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_BUILTIN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsBuiltin.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_BUILTIN_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not a built-in data type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_BUILTIN_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsBuiltin_v<T>, "Non-built-in type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_BUILTIN_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is a built-in data type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_BUILTIN_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsBuiltin_v<T>, "Built-in type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
