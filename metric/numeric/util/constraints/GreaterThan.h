// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_GREATERTHAN_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_GREATERTHAN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/HasGreaterThan.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_HAVE_GREATER_THAN_COMPARISON CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data types \a T1 and \a T2 cannot be compared in a greater-than comparison
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_GREATER_THAN_COMPARISON(T1, T2)                                            \
	static_assert(::mtrc::numeric::HasGreaterThan_v<T1, T2>, "Types without greater-than comparison detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_HAVE_GREATER_THAN_COMPARISON CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data types \a T1 and \a T2 cannot be compared in a greater-than comparison a
// compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_HAVE_GREATER_THAN_COMPARISON(T1, T2)                                        \
	static_assert(!::mtrc::numeric::HasGreaterThan_v<T1, T2>, "Type with greater-than comparison detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
