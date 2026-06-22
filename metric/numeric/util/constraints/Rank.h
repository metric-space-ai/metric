// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_RANK_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_RANK_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/Rank.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_HAVE_RANK CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the rank of a data type.
// \ingroup constraints
//
// In case the type \a T doesn't have a rank of \a N, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_RANK(T, N)                                                                 \
	static_assert(::mtrc::numeric::Rank_v<T> == N, "Invalid rank detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_HAVE_RANK CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the rank of a data type.
// \ingroup constraints
//
// In case the type \a T has a rank of \a N, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_HAVE_RANK(T, N)                                                             \
	static_assert(::mtrc::numeric::Rank_v<T> != N, "Invalid rank detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
