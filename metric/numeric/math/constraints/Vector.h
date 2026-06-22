// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECTOR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVector.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECTOR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a vector type (i.e. a type that is derived from the
// Vector base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTOR_TYPE(T)                                                               \
	static_assert(::mtrc::numeric::IsVector_v<T>, "Non-vector type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECTOR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a vector type (i.e. a type that is derived from the Vector
// base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECTOR_TYPE(T)                                                           \
	static_assert(!::mtrc::numeric::IsVector_v<T>, "Vector type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
