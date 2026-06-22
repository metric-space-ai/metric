// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_ZERO_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_ZERO_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsZero.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_ZERO_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a zero vector or matrix type, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_ZERO_TYPE(T)                                                                 \
	static_assert(::mtrc::numeric::IsZero_v<T>, "Non-zero type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_ZERO_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a zero vector or matrix type, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ZERO_TYPE(T)                                                             \
	static_assert(!::mtrc::numeric::IsZero_v<T>, "Zero type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
