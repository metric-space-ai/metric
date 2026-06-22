// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_POD_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_POD_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsPod.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_POD CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is not a POD type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_POD(T) static_assert(::mtrc::numeric::IsPod_v<T>, "Non-POD type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_POD CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is a POD type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_POD(T) static_assert(!::mtrc::numeric::IsPod_v<T>, "POD type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
