// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_POINTER_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_POINTER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsPointer.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_POINTER_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not a pointer type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_POINTER_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsPointer_v<T>, "Non-pointer type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_POINTER_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not a pointer type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_POINTER_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsPointer_v<T>, "Pointer type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
