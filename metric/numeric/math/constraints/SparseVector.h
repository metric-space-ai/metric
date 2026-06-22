// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SPARSEVECTOR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SPARSEVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSparseVector.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SPARSE_VECTOR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a sparse vector type (i.e. a type that is derived
// from the SparseVector base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsSparseVector_v<T>, "Non-sparse vector type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SPARSE_VECTOR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a sparse vector type (i.e. a type that is derived from
// the SparseVector base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SPARSE_VECTOR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsSparseVector_v<T>, "Sparse vector type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
