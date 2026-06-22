// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_ARITHMETIC_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_ARITHMETIC_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsArithmetic.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_ARITHMETIC_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not an arithmetic data type, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_ARITHMETIC_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsArithmetic_v<T>, "Non-arithmetic type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_ARITHMETIC_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is an arithmetic data type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ARITHMETIC_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsArithmetic_v<T>, "Arithmetic type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
