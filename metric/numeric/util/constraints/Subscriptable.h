// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_SUBSCRIPTABLE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_SUBSCRIPTABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SUBSCRIBTABLE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is not subscriptable, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBSCRIPTABLE(T)                                                             \
	static_assert(sizeof(std::declval<T>()[0]) > 0UL, "Non-subscriptable type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_BE_SUBSCRIBTABLE_AS_DECAYABLE_POINTER CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is not a subscriptable pointer, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBSCRIPTABLE_AS_DECAYABLE_POINTER(T)                                        \
	static_assert(sizeof(0 [std::declval<T>()]) > 0UL, "Non-subscriptable pointer detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
