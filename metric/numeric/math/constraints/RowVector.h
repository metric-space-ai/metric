// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_ROWVECTOR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_ROWVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsRowVector.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_ROW_VECTOR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a row dense or sparse vector type (i.e. a vector type
// whose transposition flag is set to mtrc::numeric::rowVector) a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_VECTOR_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsRowVector_v<T>, "Non-row vector type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_ROW_VECTOR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a row dense or sparse vector type (i.e. a vector type
// whose transposition flag is set to mtrc::numeric::rowVector) a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ROW_VECTOR_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsRowVector_v<T>, "Row vector type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
