// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_EMPTY_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_EMPTY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsEmpty.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_EMPTY CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is not an empty type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_EMPTY(T)                                                                     \
	static_assert(::mtrc::numeric::IsEmpty_v<T>, "Non-empty type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_EMPTY CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is an empty type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_EMPTY(T)                                                                 \
	static_assert(!::mtrc::numeric::IsEmpty_v<T>, "Empty type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
