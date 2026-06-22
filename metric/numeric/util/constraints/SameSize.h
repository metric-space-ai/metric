// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_SAMESIZE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_SAMESIZE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/HaveSameSize.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_HAVE_SAME_SIZE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the size of two data types.
// \ingroup constraints
//
// In case the types \a T1 and \a T2 don't have the same size, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_SAME_SIZE(T1, T2)                                                          \
	static_assert(::mtrc::numeric::HaveSameSize_v<T1, T2>, "Non-matching sizes detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_HAVE_SAME_SIZE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the size of two data types.
// \ingroup constraints
//
// In case the types \a T1 and \a T2 have the same size, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_HAVE_SAME_SIZE(T1, T2)                                                      \
	static_assert(!::mtrc::numeric::HaveSameSize_v<T1, T2>, "Matching sizes detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
