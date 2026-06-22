// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_ROUND_H
#define METRIC_NUMERIC_MATH_SIMD_ROUND_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/simd/BasicTypes.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/system/Vectorization.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  32-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Computes the nearest integers for a vector of single precision floating point values.
// \ingroup simd
//
// \param a The vector of single precision floating point values.
// \return The resulting vector.
//
// This operation is only available for SSE4.1, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat round(const SIMDf32<T> &a) noexcept
#if METRIC_NUMERIC_AVX512F_MODE
{
	return _mm512_roundscale_ps((*a).eval().value, 0);
}
#elif METRIC_NUMERIC_MIC_MODE
{
	return _mm512_round_ps((*a).eval().value, (_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC), _MM_EXPADJ_NONE);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_round_ps((*a).eval().value, (_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
}
#elif METRIC_NUMERIC_SSE4_MODE
{
	return _mm_round_ps((*a).eval().value, (_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
}
#else
	= delete;
#endif
//*************************************************************************************************

//=================================================================================================
//
//  64-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Computes the nearest integers for a vector of double precision floating point values.
// \ingroup simd
//
// \param a The vector of double precision floating point values.
// \return The resulting vector.
//
// This operation is only available for SSE4.1, AVX, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble round(const SIMDf64<T> &a) noexcept
#if METRIC_NUMERIC_AVX512F_MODE
{
	return _mm512_roundscale_pd((*a).eval().value, 0);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_round_pd((*a).eval().value, (_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
}
#elif METRIC_NUMERIC_SSE4_MODE
{
	return _mm_round_pd((*a).eval().value, (_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
