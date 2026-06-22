// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SIMDCOMBINABLE_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SIMDCOMBINABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSIMDCombinable.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SIMD_COMBINABLE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data types \a T1 and \a T2 are not SIMD combinable (i.e. do not have a
// common SIMD interface), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_COMBINABLE_TYPES(T1, T2)                                                \
	static_assert(::mtrc::numeric::IsSIMDCombinable_v<T1, T2>, "Non-SIMD combinable types detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SIMD_COMBINABLE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data types \a T1 and \a T2 are SIMD combinable (i.e. do not have a common
// SIMD interface), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SIMD_COMBINABLE_TYPES(T1, T2)                                            \
	static_assert(!::mtrc::numeric::IsSIMDCombinable_v<T1, T2>, "SIMD combinable types detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
