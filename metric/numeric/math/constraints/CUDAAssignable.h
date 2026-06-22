// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_CUDAASSIGNABLE_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_CUDAASSIGNABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsCUDAAssignable.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_CUDA_ASSIGNABLE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not CUDA-assignable (i.e. does not provide the according
// CUDA member constants or returns \a false), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_CUDA_ASSIGNABLE(T)                                                           \
	static_assert(::mtrc::numeric::IsCUDAAssignable_v<T>, "Non-CUDA assignable type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_CUDA_ASSIGNABLE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is CUDA-assignable (i.e. does provide the according CUDA
// member constants and returns \a true), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_CUDA_ASSIGNABLE(T)                                                       \
	static_assert(!::mtrc::numeric::IsCUDAAssignable_v<T>, "CUDA-assignable type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
