// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_TRANSPOSEFLAG_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_TRANSPOSEFLAG_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVector.h>
#include <metric/numeric/math/typetraits/TransposeFlag.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECTOR_WITH_TRANSPOSE_FLAG CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a dense or sparse vector type and in case the
// transpose flag of the given dense or sparse vector type \a T is not set to \a TF, a
// compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTOR_WITH_TRANSPOSE_FLAG(T, TF)                                            \
	static_assert(::mtrc::numeric::IsVector_v<T> && ::mtrc::numeric::TransposeFlag_v<T> == TF,                     \
				  "Invalid transpose flag detected")
//*************************************************************************************************

//=================================================================================================
//
//  VECTORS_MUST_HAVE_SAME_TRANSPOSE_FLAG CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case either of the two given data types \a T1 or \a T2 is not a vector type and in case
// the transpose flags of both vector types don't match, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_VECTORS_MUST_HAVE_SAME_TRANSPOSE_FLAG(T1, T2)                                        \
	static_assert(::mtrc::numeric::IsVector_v<T1> && ::mtrc::numeric::IsVector_v<T2> &&                            \
					  ::mtrc::numeric::TransposeFlag_v<T1> == ::mtrc::numeric::TransposeFlag_v<T2>,                \
				  "Invalid transpose flag detected")
//*************************************************************************************************

//=================================================================================================
//
//  VECTORS_MUST_HAVE_DIFFERENT_TRANSPOSE_FLAG CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case either of the two given data types \a T1 or \a T2 is not a vector type and in case
// the transpose flags of both vector types does match, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_VECTORS_MUST_HAVE_DIFFERENT_TRANSPOSE_FLAG(T1, T2)                                   \
	static_assert(::mtrc::numeric::IsVector_v<T1> && ::mtrc::numeric::IsVector_v<T2> &&                            \
					  ::mtrc::numeric::TransposeFlag_v<T1> != ::mtrc::numeric::TransposeFlag_v<T2>,                \
				  "Invalid transpose flag detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
