// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_STATIC_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_STATIC_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsStatic.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_STATIC_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a static data type, i.e. a vector or matrix with
// dimensions fixed at compile time, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_STATIC_TYPE(T)                                                               \
	static_assert(::mtrc::numeric::IsStatic_v<T>, "Non-static type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_STATIC_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a static data type, i.e. a vector or matrix with dimensions
// fixed at compile time, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STATIC_TYPE(T)                                                           \
	static_assert(!::mtrc::numeric::IsStatic_v<T>, "Static type dected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
