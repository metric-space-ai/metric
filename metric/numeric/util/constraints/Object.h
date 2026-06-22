// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_OBJECT_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_OBJECT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsObject.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_OBJECT_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not an object type (i.e., everything except references,
// \a void, and function types), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_OBJECT_TYPE(T)                                                               \
	static_assert(::mtrc::numeric::IsObject_v<T>, "Non-object type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_OBJECT_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not a object type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_OBJECT_TYPE(T)                                                           \
	static_assert(!::mtrc::numeric::IsObject_v<T>, "Object type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
