// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_UNIFORM_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_UNIFORM_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsUniform.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_UNIFORM_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a uniform vector or matrix type, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_UNIFORM_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsUniform_v<T>, "Non-uniform type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_UNIFORM_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a uniform vector or matrix type, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UNIFORM_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsUniform_v<T>, "Uniform type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
