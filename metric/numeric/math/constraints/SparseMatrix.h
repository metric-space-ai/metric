// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SPARSEMATRIX_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SPARSEMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSparseMatrix.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SPARSE_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a sparse matrix type (i.e. whether \a T is derived
// from the SparseMatrix base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsSparseMatrix_v<T>, "Non-sparse matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SPARSE_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a sparse matrix type (i.e. whether \a T is derived from
// the SparseMatrix base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SPARSE_MATRIX_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsSparseMatrix_v<T>, "Sparse matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
