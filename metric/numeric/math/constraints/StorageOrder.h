// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_STORAGEORDER_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_STORAGEORDER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/StorageOrder.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATRIX_WITH_STORAGE_ORDER CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a dense or sparse matrix type and in case the
// storage order of the given dense or sparse vector type \a T is not set to \a SO, a
// compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATRIX_WITH_STORAGE_ORDER(T, SO)                                             \
	static_assert(::mtrc::numeric::IsMatrix_v<T> && ::mtrc::numeric::StorageOrder_v<T> == SO,                      \
				  "Invalid storage order detected")
//*************************************************************************************************

//=================================================================================================
//
//  MATRICES_MUST_HAVE_SAME_STORAGE_ORDER CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case either of the two given data types \a T1 or \a T2 is not a matrix type and in case
// the storage order of both matrix types doesn't match, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MATRICES_MUST_HAVE_SAME_STORAGE_ORDER(T1, T2)                                        \
	static_assert(::mtrc::numeric::IsMatrix_v<T1> && ::mtrc::numeric::IsMatrix_v<T2> &&                            \
					  ::mtrc::numeric::StorageOrder_v<T1> == ::mtrc::numeric::StorageOrder_v<T2>,                  \
				  "Invalid storage order failed")
//*************************************************************************************************

//=================================================================================================
//
//  MATRICES_MUST_HAVE_DIFFERENT_STORAGE_ORDER CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case either of the two given data types \a T1 or \a T2 is not a matrix type and in case
// the storage order of both matrix types does match, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MATRICES_MUST_HAVE_DIFFERENT_STORAGE_ORDER(T1, T2)                                   \
	static_assert(::mtrc::numeric::IsMatrix_v<T1> && ::mtrc::numeric::IsMatrix_v<T2> &&                            \
					  ::mtrc::numeric::StorageOrder_v<T1> != ::mtrc::numeric::StorageOrder_v<T2>,                  \
				  "Invalid storage order detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
