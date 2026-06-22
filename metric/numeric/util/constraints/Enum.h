// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_ENUM_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_ENUM_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsEnum.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_ENUM_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not an enumeration type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_ENUM_TYPE(T)                                                                 \
	static_assert(::mtrc::numeric::IsEnum_v<T>, "Non-enum type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_ENUM_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is an enumeration type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ENUM_TYPE(T)                                                             \
	static_assert(!::mtrc::numeric::IsEnum_v<T>, "Enum type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
