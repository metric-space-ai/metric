// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_CLASS_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_CLASS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsClass.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_CLASS_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not a user-defined, non-built-in class type, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_CLASS_TYPE(T)                                                                \
	static_assert(::mtrc::numeric::IsClass_v<T>, "Non-class type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_CLASS_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is a user-defined, non-built-in class type, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_CLASS_TYPE(T)                                                            \
	static_assert(!::mtrc::numeric::IsClass_v<T>, "Class type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
