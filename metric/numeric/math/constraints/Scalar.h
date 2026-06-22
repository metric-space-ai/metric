// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SCALAR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SCALAR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsScalar.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SCALAR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a scalar type (i.e. a vector, matrix, reference or
// pointer type), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SCALAR_TYPE(T)                                                               \
	static_assert(::mtrc::numeric::IsScalar_v<T>, "Non-scalar type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SCALAR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a scalar type (i.e. neither a vector type, nor a matrix,
// reference or pointer type), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SCALAR_TYPE(T)                                                           \
	static_assert(!::mtrc::numeric::IsScalar_v<T>, "Scalar type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
