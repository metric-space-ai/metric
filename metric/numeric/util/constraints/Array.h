// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_ARRAY_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_ARRAY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsArray.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_ARRAY_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is no array type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_ARRAY_TYPE(T)                                                                \
	static_assert(::mtrc::numeric::IsArray_v<T>, "Non-array type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_ARRAY_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is an array type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ARRAY_TYPE(T)                                                            \
	static_assert(!::mtrc::numeric::IsArray_v<T>, "Array type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
