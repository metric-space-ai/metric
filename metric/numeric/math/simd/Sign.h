// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_SIGN_H
#define METRIC_NUMERIC_MATH_SIMD_SIGN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/simd/BasicTypes.h>
#include <metric/numeric/system/Compiler.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/system/Vectorization.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  8-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Sign function for a vector of 8-bit signed integral values.
// \ingroup simd
//
// \param a The vector of 8-bit unsigned integral values.
// \return 1 if the value is greater than zero, 0 if it is zero, and -1 if it is less than zero.
//
// The function returns a vector of 8-bit signed integral values, each of which is set to 1 if
// the corresponding value in \a a is greater than zero, to 0 if the corresponding value in \a a
// is zero, and to -1 if the corresponding value in \a a is less than zero. This operation is only
// available for SSSE3, AVX2, and AVX-512BW.
*/
METRIC_NUMERIC_ALWAYS_INLINE SIMDint8 sign(const SIMDint8 &a) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	const __m512i zero(_mm512_setzero_si512());
	const __mmask64 mask1(_mm512_cmplt_epi8_mask(zero, a.value));
	const __mmask64 mask2(_mm512_cmplt_epi8_mask(a.value, zero));
	const __m512i xmm1(_mm512_mask_blend_epi8(mask1, zero, _mm512_set1_epi8(1)));
	return _mm512_mask_blend_epi8(mask2, xmm1, _mm512_set1_epi8(-1));
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_sign_epi8(_mm256_set1_epi8(1), a.value);
}
#elif METRIC_NUMERIC_SSSE3_MODE
{
	return _mm_sign_epi8(_mm_set1_epi8(1), a.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//=================================================================================================
//
//  16-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Sign function for a vector of 16-bit signed integral values.
// \ingroup simd
//
// \param a The vector of 16-bit unsigned integral values.
// \return 1 if the value is greater than zero, 0 if it is zero, and -1 if it is less than zero.
//
// The function returns a vector of 16-bit signed integral values, each of which is set to 1 if
// the corresponding value in \a a is greater than zero, to 0 if the corresponding value in \a a
// is zero, and to -1 if the corresponding value in \a a is less than zero. This operation is only
// available for SSSE3, AVX2, and AVX-512BW.
*/
METRIC_NUMERIC_ALWAYS_INLINE SIMDint16 sign(const SIMDint16 &a) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	const __m512i zero(_mm512_setzero_si512());
	const __mmask32 mask1(_mm512_cmplt_epi16_mask(zero, a.value));
	const __mmask32 mask2(_mm512_cmplt_epi16_mask(a.value, zero));
	const __m512i xmm1(_mm512_mask_blend_epi16(mask1, zero, _mm512_set1_epi16(1)));
	return _mm512_mask_blend_epi16(mask2, xmm1, _mm512_set1_epi16(-1));
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_sign_epi16(_mm256_set1_epi16(1), a.value);
}
#elif METRIC_NUMERIC_SSSE3_MODE
{
	return _mm_sign_epi16(_mm_set1_epi16(1), a.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//=================================================================================================
//
//  32-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Sign function for a vector of 32-bit signed integral values.
// \ingroup simd
//
// \param a The vector of 32-bit unsigned integral values.
// \return 1 if the value is greater than zero, 0 if it is zero, and -1 if it is less than zero.
//
// The function returns a vector of 32-bit signed integral values, each of which is set to 1 if
// the corresponding value in \a a is greater than zero, to 0 if the corresponding value in \a a
// is zero, and to -1 if the corresponding value in \a a is less than zero. This operation is only
// available for SSSE3, AVX2, and AVX-512F.
*/
METRIC_NUMERIC_ALWAYS_INLINE SIMDint32 sign(const SIMDint32 &a) noexcept
#if METRIC_NUMERIC_AVX512F_MODE
{
	const __m512i zero(_mm512_setzero_si512());
	const __mmask16 mask1(_mm512_cmplt_epi32_mask(zero, a.value));
	const __mmask16 mask2(_mm512_cmplt_epi32_mask(a.value, zero));
	const __m512i xmm1(_mm512_mask_blend_epi32(mask1, zero, _mm512_set1_epi32(1)));
	return _mm512_mask_blend_epi32(mask2, xmm1, _mm512_set1_epi32(-1));
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_sign_epi32(_mm256_set1_epi32(1), a.value);
}
#elif METRIC_NUMERIC_SSSE3_MODE
{
	return _mm_sign_epi32(_mm_set1_epi32(1), a.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//=================================================================================================
//
//  64-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Sign function for a vector of 64-bit signed integral values.
// \ingroup simd
//
// \param a The vector of 64-bit unsigned integral values.
// \return 1 if the value is greater than zero, 0 if it is zero, and -1 if it is less than zero.
//
// The function returns a vector of 64-bit signed integral values, each of which is set to 1 if
// the corresponding value in \a a is greater than zero, to 0 if the corresponding value in \a a
// is zero, and to -1 if the corresponding value in \a a is less than zero. This operation is only
// available for AVX-512F.
*/
METRIC_NUMERIC_ALWAYS_INLINE SIMDint64 sign(const SIMDint64 &a) noexcept
#if METRIC_NUMERIC_AVX512F_MODE
{
	const __m512i zero(_mm512_setzero_si512());
	const __mmask8 mask1(_mm512_cmplt_epi64_mask(zero, a.value));
	const __mmask8 mask2(_mm512_cmplt_epi64_mask(a.value, zero));
	const __m512i xmm1(_mm512_mask_blend_epi64(mask1, zero, _mm512_set1_epi64(1L)));
	return _mm512_mask_blend_epi64(mask2, xmm1, _mm512_set1_epi64(-1L));
}
#else
	= delete;
#endif
//*************************************************************************************************

//=================================================================================================
//
//  32-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Sign function for a vector of single precision floating point values.
// \ingroup simd
//
// \param a The vector of single precision floating point values.
// \return 1 if the value is greater than zero, 0 if it is zero, and -1 if it is less than zero.
//
// The function returns a vector of single precision floating point values, each of which is set
// to 1 if the corresponding value in \a a is greater than zero, to 0 if the corresponding value
// in \a a is zero, and to -1 if the corresponding value in \a a is less than zero. This operation
// is only available for SSE4, AVX, MIC, and AVX-512F.
*/
METRIC_NUMERIC_ALWAYS_INLINE SIMDfloat sign(const SIMDfloat &a) noexcept
#if (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE) && METRIC_NUMERIC_GNU_COMPILER
	= delete;
#elif METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	const __m512 zero(_mm512_setzero_ps());
	const __mmask16 mask1(_mm512_cmplt_ps_mask(zero, a.value));
	const __mmask16 mask2(_mm512_cmplt_ps_mask(a.value, zero));
	const __m512 xmm1(_mm512_mask_blend_ps(mask1, a.value, _mm512_set1_ps(1.0F)));
	return _mm512_mask_blend_ps(mask2, xmm1, _mm512_set1_ps(-1.0F));
}
#elif METRIC_NUMERIC_AVX_MODE
{
	const __m256 zero(_mm256_setzero_ps());
	const __m256 mask1(_mm256_cmp_ps(zero, a.value, _CMP_LT_OQ));
	const __m256 mask2(_mm256_cmp_ps(a.value, zero, _CMP_LT_OQ));
	const __m256 xmm1(_mm256_blendv_ps(a.value, _mm256_set1_ps(1.0F), mask1));
	return _mm256_blendv_ps(xmm1, _mm256_set1_ps(-1.0F), mask2);
}
#elif METRIC_NUMERIC_SSE4_MODE
{
	const __m128 zero(_mm_setzero_ps());
	const __m128 mask1(_mm_cmplt_ps(zero, a.value));
	const __m128 mask2(_mm_cmplt_ps(a.value, zero));
	const __m128 xmm1(_mm_blendv_ps(a.value, _mm_set1_ps(1.0F), mask1));
	return _mm_blendv_ps(xmm1, _mm_set1_ps(-1.0F), mask2);
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
/*!\brief Sign function for a vector of double precision floating point values.
// \ingroup simd
//
// \param a The vector of double precision floating point values.
// \return 1 if the value is greater than zero, 0 if it is zero, and -1 if it is less than zero.
//
// The function returns a vector of double precision floating point values, each of which is set
// to 1 if the corresponding value in \a a is greater than zero, to 0 if the corresponding value
// in \a a is zero, and to -1 if the corresponding value in \a a is less than zero. This operation
// is only available for SSE4, AVX, MIC, and AVX-512F.
*/
METRIC_NUMERIC_ALWAYS_INLINE SIMDdouble sign(const SIMDdouble &a) noexcept
#if (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE) && METRIC_NUMERIC_GNU_COMPILER
	= delete;
#elif METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	const __m512d zero(_mm512_setzero_pd());
	const __mmask8 mask1(_mm512_cmplt_pd_mask(zero, a.value));
	const __mmask8 mask2(_mm512_cmplt_pd_mask(a.value, zero));
	const __m512d xmm1(_mm512_mask_blend_pd(mask1, a.value, _mm512_set1_pd(1.0)));
	return _mm512_mask_blend_pd(mask2, xmm1, _mm512_set1_pd(-1.0));
}
#elif METRIC_NUMERIC_AVX_MODE
{
	const __m256d zero(_mm256_setzero_pd());
	const __m256d mask1(_mm256_cmp_pd(zero, a.value, _CMP_LT_OQ));
	const __m256d mask2(_mm256_cmp_pd(a.value, zero, _CMP_LT_OQ));
	const __m256d xmm1(_mm256_blendv_pd(a.value, _mm256_set1_pd(1.0), mask1));
	return _mm256_blendv_pd(xmm1, _mm256_set1_pd(-1.0), mask2);
}
#elif METRIC_NUMERIC_SSE4_MODE
{
	const __m128d zero(_mm_setzero_pd());
	const __m128d mask1(_mm_cmplt_pd(zero, a.value));
	const __m128d mask2(_mm_cmplt_pd(a.value, zero));
	const __m128d xmm1(_mm_blendv_pd(a.value, _mm_set1_pd(1.0), mask1));
	return _mm_blendv_pd(xmm1, _mm_set1_pd(-1.0), mask2);
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
