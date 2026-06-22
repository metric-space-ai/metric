// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SIMDPACK_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SIMDPACK_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSIMDPack.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SIMD_PACK CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a SIMD pack, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T)                                                                 \
	static_assert(::mtrc::numeric::IsSIMDPack_v<T>, "Non-SIMD pack detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SIMD_PACK CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a SIMD pack, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SIMD_PACK(T)                                                             \
	static_assert(!::mtrc::numeric::IsSIMDPack_v<T>, "SIMD pack detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
