// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_VOID_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_VOID_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsVoid.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VOID CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is not \a void, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VOID(T)                                                                      \
	static_assert(::mtrc::numeric::IsVoid_v<T>, "Non-void type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VOID CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is \a void, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VOID(T)                                                                  \
	static_assert(!::mtrc::numeric::IsVoid_v<T>, "Void type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
