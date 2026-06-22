// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SPARSEELEMENT_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SPARSEELEMENT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSparseElement.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SPARSE_ELEMENT_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a sparse element type (i.e. a type derived from the
// SparseElement base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_ELEMENT_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsSparseElement_v<T>, "Non-sparse element type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SPARSE_ELEMENT_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a sparse element type (i.e. a type derived from the
// SparseElement base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SPARSE_ELEMENT_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsSparseElement_v<T>, "Sparse element type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
