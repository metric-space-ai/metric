// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SIMDENABLED_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SIMDENABLED_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSIMDEnabled.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SIMD_ENABLED CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not SIMD-enabled (i.e. does not provide the according
// SIMD member functions), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_ENABLED(T)                                                              \
	static_assert(::mtrc::numeric::IsSIMDEnabled_v<T>, "Non-SIMD enabled type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SIMD_ENABLED CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is SIMD-enabled (i.e. does provide the according SIMD member
// functions), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SIMD_ENABLED(T)                                                          \
	static_assert(!::mtrc::numeric::IsSIMDEnabled_v<T>, "SIMD enabled type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
