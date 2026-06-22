// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_DENSEVECTOR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_DENSEVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDenseVector.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DENSE_VECTOR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a dense vector type (i.e. a type that is derived from
// the DenseVector base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(T)                                                         \
	static_assert(::mtrc::numeric::IsDenseVector_v<T>, "Non-dense vector type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DENSE_VECTOR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a dense vector type (i.e. a type that is derived from
// the DenseVector base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DENSE_VECTOR_TYPE(T)                                                     \
	static_assert(!::mtrc::numeric::IsDenseVector_v<T>, "Dense vector type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
