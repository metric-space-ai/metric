// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATRIX_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatrix.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix type (i.e. a type that is derived from the
// Matrix base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATRIX_TYPE(T)                                                               \
	static_assert(::mtrc::numeric::IsMatrix_v<T>, "Non-matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix type (i.e. a type that is derived from the Matrix
// base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATRIX_TYPE(T)                                                           \
	static_assert(!::mtrc::numeric::IsMatrix_v<T>, "Matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
