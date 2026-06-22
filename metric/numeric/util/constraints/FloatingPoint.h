// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_FLOATINGPOINT_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_FLOATINGPOINT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsFloatingPoint.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_FLOATING_POINT_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not a floating point data type, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_FLOATING_POINT_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsFloatingPoint_v<T>, "Non-floating point type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_FLOATING_POINT_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is a floating point data type, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_FLOATING_POINT_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsFloatingPoint_v<T>, "Floating point type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
