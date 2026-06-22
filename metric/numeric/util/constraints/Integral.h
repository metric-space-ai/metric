// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_INTEGRAL_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_INTEGRAL_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsIntegral.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_INTEGRAL_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not an integral data type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_INTEGRAL_TYPE(T)                                                             \
	static_assert(::mtrc::numeric::IsIntegral_v<T>, "Non-integral type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_INTEGRAL_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is an integral data type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_INTEGRAL_TYPE(T)                                                         \
	static_assert(!::mtrc::numeric::IsIntegral_v<T>, "Integral type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
