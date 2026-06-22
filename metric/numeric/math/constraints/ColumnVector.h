// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_COLUMNVECTOR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_COLUMNVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsColumnVector.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_COLUMN_VECTOR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a column dense or sparse vector type (i.e. a vector
// type whose transposition flag is set to mtrc::numeric::columnVector) a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsColumnVector_v<T>, "Non-column vector type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_COLUMN_VECTOR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a column dense or sparse vector type (i.e. a vector type
// whose transposition flag is set to mtrc::numeric::columnVector) a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COLUMN_VECTOR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsColumnVector_v<T>, "Column vector type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
