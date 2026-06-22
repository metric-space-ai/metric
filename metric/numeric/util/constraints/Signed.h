// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_SIGNED_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_SIGNED_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsSigned.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SIGNED_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not an signed integral data type, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIGNED_TYPE(T)                                                               \
	static_assert(::mtrc::numeric::IsSigned_v<T>, "Non-signed type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SIGNED_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is an signed integral data type, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SIGNED_TYPE(T)                                                           \
	static_assert(!::mtrc::numeric::IsSigned_v<T>, "Signed type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
