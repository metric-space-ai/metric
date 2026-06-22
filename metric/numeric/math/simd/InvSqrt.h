// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_INVSQRT_H
#define METRIC_NUMERIC_MATH_SIMD_INVSQRT_H
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
/*!\brief Computes the inverse square root for a vector of single precision floating point values.
// \ingroup simd
//
// \param a The vector of single precision floating point values.
// \return The resulting vector.
//
// This operation is only available via the SVML for SSE, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat invsqrt(const SIMDf32<T> &a) noexcept
#if METRIC_NUMERIC_SVML_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return _mm512_invsqrt_ps((*a).eval().value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX_MODE
{
	return _mm256_invsqrt_ps((*a).eval().value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_SSE_MODE
{
	return _mm_rsqrt_ps((*a).eval().value);
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
/*!\brief Computes the inverse square root for a vector of double precision floating point values.
// \ingroup simd
//
// \param a The vector of double precision floating point values.
// \return The resulting vector.
//
// This operation is only available via the SVML for SSE, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble invsqrt(const SIMDf64<T> &a) noexcept
#if METRIC_NUMERIC_SVML_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return _mm512_invsqrt_pd((*a).eval().value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX_MODE
{
	return _mm256_invsqrt_pd((*a).eval().value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_SSE_MODE
{
	return _mm_invsqrt_pd((*a).eval().value);
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
