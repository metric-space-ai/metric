// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_DESTRUCTIBLE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_DESTRUCTIBLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsDestructible.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DESTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T cannot be destroyed via its destructor, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DESTRUCTIBLE_TYPE(T)                                                         \
	static_assert(::mtrc::numeric::IsDestructible_v<T>, "Non-destructible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DESTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T can be destroyed via its destructor, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DESTRUCTIBLE_TYPE(T)                                                     \
	static_assert(!::mtrc::numeric::IsDestructible_v<T>, "Destructible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_BE_NOTHROW_DESTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T cannot be destroyed via a noexcept destructor, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_NOTHROW_DESTRUCTIBLE_TYPE(T)                                                 \
	static_assert(::mtrc::numeric::IsNothrowDestructible_v<T>, "Non-noexcept destructible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_NOTHROW_DESTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T can be destroyed via a noexcept destructor, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_NOTHROW_DESTRUCTIBLE_TYPE(T)                                             \
	static_assert(!::mtrc::numeric::IsNothrowDestructible_v<T>, "Noexcept destructible type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
