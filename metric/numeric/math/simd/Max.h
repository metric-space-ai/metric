// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_MAX_H
#define METRIC_NUMERIC_MATH_SIMD_MAX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/simd/BasicTypes.h>
#include <metric/numeric/math/typetraits/IsSIMDPack.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/system/Vectorization.h>
#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  8-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Componentwise maximum of two vectors of 8-bit signed integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The resulting vector.
//
// This operation is only available for SSE4, AVX2, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint8 max(const SIMDint8 &a, const SIMDint8 &b) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_max_epi8((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_max_epi8((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE4_MODE
{
	return _mm_max_epi8((*a).value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Componentwise maximum of two vectors of 8-bit unsigned integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The resulting vector.
//
// This operation is only available for SSE2, AVX2, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint8 max(const SIMDuint8 &a, const SIMDuint8 &b) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_max_epu8((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_max_epu8((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_max_epu8((*a).value, (*b).value);
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
/*!\brief Componentwise maximum of two vectors of 16-bit signed integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The resulting vector.
//
// This operation is only available for SSE2, AVX2, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint16 max(const SIMDint16 &a, const SIMDint16 &b) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_max_epi16((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_max_epi16((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_max_epi16((*a).value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Componentwise maximum of two vectors of 16-bit unsigned integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The resulting vector.
//
// This operation is only available for SSE4, AVX2, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint16 max(const SIMDuint16 &a, const SIMDuint16 &b) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_max_epu16((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_max_epu16((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE4_MODE
{
	return _mm_max_epu16((*a).value, (*b).value);
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
/*!\brief Componentwise maximim of two vectors of 32-bit signed integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The resulting vector.
//
// This operation is only available for SSE4, AVX2, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint32 max(const SIMDint32 &a, const SIMDint32 &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_max_epi32((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_max_epi32((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE4_MODE
{
	return _mm_max_epi32((*a).value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Componentwise maximum of two vectors of 32-bit unsigned integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The resulting vector.
//
// This operation is only available for SSE4, AVX2, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint32 max(const SIMDuint32 &a, const SIMDuint32 &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_max_epu32((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_max_epu32((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE4_MODE
{
	return _mm_max_epu32((*a).value, (*b).value);
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
/*!\brief Componentwise maximum of two vectors of single precision floating point SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The resulting vector.
//
// This operation is only available for SSE, AVX, MIC, and AVX-512.
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat max(const SIMDf32<T1> &a, const SIMDf32<T2> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_max_ps((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_max_ps((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_max_ps((*a).eval().value, (*b).eval().value);
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
/*!\brief Componentwise maximum of two vectors of double precision floating point SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The resulting vector.
//
// This operation is only available for SSE2, AVX, MIC, and AVX-512.
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble max(const SIMDf64<T1> &a, const SIMDf64<T2> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_max_pd((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_max_pd((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_max_pd((*a).eval().value, (*b).eval().value);
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
