// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_COS_H
#define METRIC_NUMERIC_MATH_SIMD_COS_H
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
/*!\brief Cosine of a vector of single precision floating point values.
// \ingroup simd
//
// \param a The vector of single precision floating point values.
// \return The resulting vector.
//
// This operation is only available via the SVML or SLEEF for SSE, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat cos(const SIMDf32<T> &a) noexcept
#if METRIC_NUMERIC_SVML_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_cos_ps((*a).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_cos_ps((*a).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_cos_ps((*a).eval().value);
}
#endif
#elif METRIC_NUMERIC_SLEEF_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return Sleef_cosf16_u10((*a).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return Sleef_cosf8_u10((*a).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return Sleef_cosf4_u10((*a).eval().value);
}
#endif
#elif METRIC_NUMERIC_XSIMD_MODE
{
	return SIMDfloat{xsimd::cos(xsimd::batch<float>{(*a).eval().value}).data};
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
/*!\brief Cosine of a vector of double precision floating point values.
// \ingroup simd
//
// \param a The vector of double precision floating point values.
// \return The resulting vector.
//
// This operation is only available via the SVML or SLEEF for SSE, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble cos(const SIMDf64<T> &a) noexcept
#if METRIC_NUMERIC_SVML_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_cos_pd((*a).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_cos_pd((*a).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_cos_pd((*a).eval().value);
}
#endif
#elif METRIC_NUMERIC_SLEEF_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return Sleef_cosd8_u10((*a).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return Sleef_cosd4_u10((*a).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return Sleef_cosd2_u10((*a).eval().value);
}
#endif
#elif METRIC_NUMERIC_XSIMD_MODE
{
	return SIMDdouble{xsimd::cos(xsimd::batch<double>{(*a).eval().value}).data};
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
