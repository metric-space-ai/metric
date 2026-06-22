// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_LOG10_H
#define METRIC_NUMERIC_MATH_SIMD_LOG10_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#if METRIC_NUMERIC_SLEEF_MODE
#include <sleef.h>
#endif
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
/*!\brief Computes the common logarithm for a vector of single precision floating point values.
// \ingroup simd
//
// \param a The vector of single precision floating point values.
// \return The resulting vector.
//
// This operation is only available via the SVML or SLEEF for SSE, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat log10(const SIMDf32<T> &a) noexcept
#if METRIC_NUMERIC_SVML_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_log10_ps((*a).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_log10_ps((*a).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_log10_ps((*a).eval().value);
}
#endif
#elif METRIC_NUMERIC_SLEEF_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return Sleef_log10f16_u10((*a).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return Sleef_log10f8_u10((*a).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return Sleef_log10f4_u10((*a).eval().value);
}
#endif
#elif METRIC_NUMERIC_XSIMD_MODE
{
	return SIMDfloat{xsimd::log10(xsimd::batch<float>{(*a).eval().value}).data};
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
/*!\brief Computes the common logarithm for a vector of double precision floating point values.
// \ingroup simd
//
// \param a The vector of double precision floating point values.
// \return The resulting vector.
//
// This operation is only available via the SVML or SLEEF for SSE, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble log10(const SIMDf64<T> &a) noexcept
#if METRIC_NUMERIC_SVML_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_log10_pd((*a).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_log10_pd((*a).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_log10_pd((*a).eval().value);
}
#endif
#elif METRIC_NUMERIC_SLEEF_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return Sleef_log10d8_u10((*a).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return Sleef_log10d4_u10((*a).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return Sleef_log10d2_u10((*a).eval().value);
}
#endif
#elif METRIC_NUMERIC_XSIMD_MODE
{
	return SIMDdouble{xsimd::log10(xsimd::batch<double>{(*a).eval().value}).data};
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
