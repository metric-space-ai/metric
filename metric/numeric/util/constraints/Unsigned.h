// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_UNSIGNED_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_UNSIGNED_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsUnsigned.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_UNSIGNED_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not an unsigned integral data type, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_UNSIGNED_TYPE(T)                                                             \
	static_assert(::mtrc::numeric::IsUnsigned_v<T>, "Non-unsigned type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_UNSIGNED_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is an unsigned integral data type, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UNSIGNED_TYPE(T)                                                         \
	static_assert(!::mtrc::numeric::IsUnsigned_v<T>, "Unsigned type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
