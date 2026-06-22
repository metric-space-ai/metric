// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_COMPARABLE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_COMPARABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsConvertible.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  POINTER_MUST_BE_COMPARABLE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the pointer relationship.
// \ingroup constraints
//
// In case \a P1 is not comparable with \a P2, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_POINTER_MUST_BE_COMPARABLE(P1, P2)                                                   \
	static_assert(::mtrc::numeric::IsConvertible_v<P1, P2> || ::mtrc::numeric::IsConvertible_v<P2, P1>,            \
				  "Incomparable pointer types detected");
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
