// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_CONVERTIBLE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_CONVERTIBLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsConvertible.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_CONVERTIBLE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the pointer relationship.
// \ingroup constraints
//
// In case \a FROM is not convertible to \a TO, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_CONVERTIBLE(FROM, TO)                                                        \
	static_assert(::mtrc::numeric::IsConvertible_v<FROM, TO>, "Inconvertible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_CONVERTIBLE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the pointer relationship.
// \ingroup constraints
//
// In case \a FROM is convertible to \a TO, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_CONVERTIBLE(FROM, TO)                                                    \
	static_assert(!::mtrc::numeric::IsConvertible_v<FROM, TO>, "Convertible type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
