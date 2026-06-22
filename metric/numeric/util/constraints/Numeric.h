// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_NUMERIC_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_NUMERIC_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsNumeric.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_NUMERIC_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not a numeric (integral or floating point) data type,
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_NUMERIC_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsNumeric_v<T>, "Non-numeric type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_NUMERIC_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is a numeric (integral or floating point) data type,
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_NUMERIC_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsNumeric_v<T>, "Numeric type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
