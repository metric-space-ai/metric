// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_ROWMAJORMATRIX_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_ROWMAJORMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsRowMajorMatrix.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_ROW_MAJOR_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a row-major dense or sparse matrix type (i.e. a matrix
// type whose storage order is set to \a false) a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE(T)                                                     \
	static_assert(::mtrc::numeric::IsRowMajorMatrix_v<T>, "Non-row-major matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_ROW_MAJOR_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a row-major dense or sparse matrix type (i.e. a matrix
// type whose storage order is set to \a false) a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ROW_MAJOR_MATRIX_TYPE(T)                                                 \
	static_assert(!::mtrc::numeric::IsRowMajorMatrix_v<T>, "Row-major matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
