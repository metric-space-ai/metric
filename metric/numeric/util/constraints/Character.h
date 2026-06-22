// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_CHARACTER_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_CHARACTER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsCharacter.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_CHARACTER_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not a character data type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_CHARACTER_TYPE(T)                                                            \
	static_assert(::mtrc::numeric::IsCharacter_v<T>, "Non-character type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_CHARACTER_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is a character data type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_CHARACTER_TYPE(T)                                                        \
	static_assert(!::mtrc::numeric::IsCharacter_v<T>, "Character type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
