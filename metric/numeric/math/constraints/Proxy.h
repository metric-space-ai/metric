// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_PROXY_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_PROXY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsProxy.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_PROXY_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a proxy type (i.e. a type derived from the Proxy class
// template), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_PROXY_TYPE(T)                                                                \
	static_assert(::mtrc::numeric::IsProxy_v<T>, "Non-proxy type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_PROXY_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a proxy type (i.e. a type derived from the Proxy class
// template), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_PROXY_TYPE(T)                                                            \
	static_assert(!::mtrc::numeric::IsProxy_v<T>, "Proxy type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
