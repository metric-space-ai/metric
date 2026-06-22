// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_VALID_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_VALID_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsValid.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VALID CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is not a valid type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VALID_TYPE(T)                                                                \
	static_assert(mtrc::numeric::IsValid_v<T>, "Invalid type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VALID CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is a valid type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VALID_TYPE(T)                                                            \
	static_assert(!mtrc::numeric::IsValid_v<T>, "Valid type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
