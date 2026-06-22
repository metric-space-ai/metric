// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_ABS_H
#define METRIC_NUMERIC_MATH_SIMD_ABS_H
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
/*!\brief Absolute value of a vector of 8-bit signed integral values.
// \ingroup simd
//
// \param a The vector of 8-bit unsigned integral values.
// \return The absolute values.
//
// This operation is only available for SSSE3, AVX2, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint8 abs(const SIMDint8 &a) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_abs_epi8(a.value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_abs_epi8(a.value);
}
#elif METRIC_NUMERIC_SSSE3_MODE
{
	return _mm_abs_epi8(a.value);
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
/*!\brief Absolute value of a vector of 16-bit signed integral values.
// \ingroup simd
//
// \param a The vector of 16-bit unsigned integral values.
// \return The absolute values.
//
// This operation is only available for SSSE3, AVX2, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint16 abs(const SIMDint16 &a) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_abs_epi16(a.value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_abs_epi16(a.value);
}
#elif METRIC_NUMERIC_SSSE3_MODE
{
	return _mm_abs_epi16(a.value);
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
/*!\brief Absolute value of a vector of 32-bit signed integral values.
// \ingroup simd
//
// \param a The vector of 32-bit unsigned integral values.
// \return The absolute values.
//
// This operation is only available for SSSE3, AVX2, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint32 abs(const SIMDint32 &a) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_abs_epi32(a.value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_abs_epi32(a.value);
}
#elif METRIC_NUMERIC_SSSE3_MODE
{
	return _mm_abs_epi32(a.value);
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
/*!\brief Absolute value of a vector of 64-bit signed integral values.
// \ingroup simd
//
// \param a The vector of 64-bit unsigned integral values.
// \return The absolute values.
//
// This operation is only available for MIC and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint64 abs(const SIMDint64 &a) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_abs_epi64(a.value);
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
/*!\brief Absolute of a vector of single precision floating point values.
// \ingroup simd
//
// \param a The vector of single precision floating point values \f$[-1..1]\f$.
// \return The resulting vector.
//
// This operation is only available for SSE2, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat abs(const SIMDf32<T> &a) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_abs_ps((*a).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	const __m256 mask(_mm256_castsi256_ps(_mm256_set1_epi32(0x80000000)));
	return _mm256_andnot_ps(mask, (*a).eval().value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	const __m128 mask(_mm_castsi128_ps(_mm_set1_epi32(0x80000000)));
	return _mm_andnot_ps(mask, (*a).eval().value);
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
/*!\brief Absolute of a vector of double precision floating point values.
// \ingroup simd
//
// \param a The vector of double precision floating point values \f$[-1..1]\f$.
// \return The resulting vector.
//
// This operation is only available for SSE2, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble abs(const SIMDf64<T> &a) noexcept
#if (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE) && METRIC_NUMERIC_GNU_COMPILER
	= delete;
#elif METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_abs_pd((*a).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	const __m256d mask(
		_mm256_castsi256_pd(_mm256_set_epi32(0x80000000, 0x0, 0x80000000, 0x0, 0x80000000, 0x0, 0x80000000, 0x0)));
	return _mm256_andnot_pd(mask, (*a).eval().value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	const __m128d mask(_mm_castsi128_pd(_mm_set_epi32(0x80000000, 0x0, 0x80000000, 0x0)));
	return _mm_andnot_pd(mask, (*a).eval().value);
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
