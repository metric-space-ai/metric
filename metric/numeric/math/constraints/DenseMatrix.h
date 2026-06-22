// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_DENSEMATRIX_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_DENSEMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDenseMatrix.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DENSE_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a dense matrix type (i.e. a type that is derived from
// the DenseMatrix base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(T)                                                         \
	static_assert(::mtrc::numeric::IsDenseMatrix_v<T>, "Non-dense matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DENSE_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a dense matrix type (i.e. a type that is derived from
// the DenseMatrix base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DENSE_MATRIX_TYPE(T)                                                     \
	static_assert(!::mtrc::numeric::IsDenseMatrix_v<T>, "Dense matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
