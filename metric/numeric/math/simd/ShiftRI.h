// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_SHIFTRI_H
#define METRIC_NUMERIC_MATH_SIMD_SHIFTRI_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/simd/BasicTypes.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/system/Vectorization.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  16-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Uniform right-shift of a vector of 16-bit signed integral SIMD values.
// \ingroup simd
//
// \param a The vector of 16-bit signed integral values to be shifted.
// \param count The number of bits to shift \f$[0..15]\f$.
// \return The result of the right-shift.
//
// This operation is only available for SSE2, AVX2, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint16 operator>>(const SIMDint16 &a, int count) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_srai_epi16(a.value, static_cast<unsigned int>(count));
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_srai_epi16(a.value, count);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_srai_epi16(a.value, count);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Uniform right-shift of a vector of 16-bit unsigned integral SIMD values.
// \ingroup simd
//
// \param a The vector of 16-bit unsigned integral values to be shifted.
// \param count The number of bits to shift \f$[0..15]\f$.
// \return The result of the right-shift.
//
// This operation is only available for SSE2, AVX2, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint16 operator>>(const SIMDuint16 &a, int count) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_srli_epi16(a.value, static_cast<unsigned int>(count));
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_srli_epi16(a.value, count);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_srli_epi16(a.value, count);
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
/*!\brief Uniform right-shift of a vector of 32-bit signed integral SIMD values.
// \ingroup simd
//
// \param a The vector of 32-bit signed integral values to be shifted.
// \param count The number of bits to shift \f$[0..31]\f$.
// \return The result of the right-shift.
//
// This operation is only available for SSE2, AVX2, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint32 operator>>(const SIMDint32 &a, int count) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_srai_epi32(a.value, static_cast<unsigned int>(count));
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_srai_epi32(a.value, count);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_srai_epi32(a.value, count);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Uniform right-shift of a vector of 32-bit unsigned integral SIMD values.
// \ingroup simd
//
// \param a The vector of 32-bit unsigned integral values to be shifted.
// \param count The number of bits to shift \f$[0..31]\f$.
// \return The result of the right-shift.
//
// This operation is only available for SSE2, AVX2, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint32 operator>>(const SIMDuint32 &a, int count) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_srli_epi32(a.value, static_cast<unsigned int>(count));
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_srli_epi32(a.value, count);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_srli_epi32(a.value, count);
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
/*!\brief Uniform right-shift of a vector of 64-bit signed integral SIMD values.
// \ingroup simd
//
// \param a The vector of 64-bit signed integral values to be shifted.
// \param count The number of bits to shift \f$[0..63]\f$.
// \return The result of the right-shift.
//
// This operation is only available for AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint64 operator>>(const SIMDint64 &a, int count) noexcept
#if METRIC_NUMERIC_AVX512F_MODE
{
	return _mm512_srai_epi64(a.value, static_cast<unsigned int>(count));
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Uniform right-shift of a vector of 64-bit unsigned integral SIMD values.
// \ingroup simd
//
// \param a The vector of 64-bit unsigned integral values to be shifted.
// \param count The number of bits to shift \f$[0..63]\f$.
// \return The result of the right-shift.
//
// This operation is only available for SSE2, AVX2 and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint64 operator>>(const SIMDuint64 &a, int count) noexcept
#if METRIC_NUMERIC_AVX512F_MODE
{
	return _mm512_srli_epi64(a.value, static_cast<unsigned int>(count));
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_srli_epi64(a.value, count);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_srli_epi64(a.value, count);
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
