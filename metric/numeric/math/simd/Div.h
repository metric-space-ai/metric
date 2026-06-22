// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_DIV_H
#define METRIC_NUMERIC_MATH_SIMD_DIV_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/simd/BasicTypes.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/system/Vectorization.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  8-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Division of two vectors of 8-bit signed integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the division.
//
// This operation is only available for AVX2 and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint8 operator/(const SIMDint8 &a, const SIMDint8 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_div_epi8(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epi8(a.value, b.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Division of two vectors of 8-bit unsigned integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the division.
//
// This operation is only available for AVX2 and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint8 operator/(const SIMDuint8 &a, const SIMDuint8 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_div_epu8(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epu8(a.value, b.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Scaling of a vector of 8-bit signed integral complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side complex values to be scaled.
// \param b The right-hand side scalars.
// \return The result of the division.
//
// This operation is only available for AVX2 and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcint8 operator/(const SIMDcint8 &a, const SIMDint8 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_div_epi8(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epi8(a.value, b.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Scaling of a vector of 8-bit unsigned integral complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side complex values to be scaled.
// \param b The right-hand side scalars.
// \return The result of the division.
//
// This operation is only available for AVX2 and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcuint8 operator/(const SIMDcuint8 &a, const SIMDuint8 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_div_epu8(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epu8(a.value, b.value);
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
/*!\brief Division of two vectors of 16-bit signed integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the division.
//
// This operation is only available for AVX2 and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint16 operator/(const SIMDint16 &a, const SIMDint16 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_div_epi16(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epi16(a.value, b.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Division of two vectors of 16-bit unsigned integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the division.
//
// This operation is only available for AVX2 and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint16 operator/(const SIMDuint16 &a, const SIMDuint16 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_div_epu16(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epu16(a.value, b.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Scaling of a vector of 16-bit signed integral complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side complex values to be scaled.
// \param b The right-hand side scalars.
// \return The result of the division.
//
// This operation is only available for AVX2 and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcint16 operator/(const SIMDcint16 &a, const SIMDint16 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_div_epi16(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epi16(a.value, b.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Scaling of a vector of 16-bit unsigned integral complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side complex values to be scaled.
// \param b The right-hand side scalars.
// \return The result of the division.
//
// This operation is only available for AVX2 and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcuint16 operator/(const SIMDcuint16 &a, const SIMDuint16 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_div_epu16(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epu16(a.value, b.value);
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
/*!\brief Division of two vectors of 32-bit signed integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the division.
//
// This operation is only available for AVX2, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint32 operator/(const SIMDint32 &a, const SIMDint32 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return _mm512_div_epi32(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epi32(a.value, b.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Division of two vectors of 32-bit unsigned integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the division.
//
// This operation is only available for AVX2, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint32 operator/(const SIMDuint32 &a, const SIMDuint32 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return _mm512_div_epu32(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epu32(a.value, b.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Scaling of a vector of 32-bit signed integral complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side complex values to be scaled.
// \param b The right-hand side scalars.
// \return The result of the division.
//
// This operation is only available for AVX2, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcint32 operator/(const SIMDcint32 &a, const SIMDint32 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return _mm512_div_epi32(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epi32(a.value, b.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Scaling of a vector of 32-bit unsigned integral complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side complex values to be scaled.
// \param b The right-hand side scalars.
// \return The result of the division.
//
// This operation is only available for AVX2, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcuint32 operator/(const SIMDcuint32 &a, const SIMDuint32 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return _mm512_div_epu32(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epu32(a.value, b.value);
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
/*!\brief Division of two vectors of 64-bit signed integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the division.
//
// This operation is only available for AVX2, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint64 operator/(const SIMDint64 &a, const SIMDint64 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return _mm512_div_epi64(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epi64(a.value, b.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Division of two vectors of 64-bit unsigned integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the division.
//
// This operation is only available for AVX2, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint64 operator/(const SIMDuint64 &a, const SIMDuint64 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return _mm512_div_epu64(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epu64(a.value, b.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Scaling of a vector of 64-bit signed integral complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side complex values to be scaled.
// \param b The right-hand side scalars.
// \return The result of the division.
//
// This operation is only available for AVX2, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcint64 operator/(const SIMDcint64 &a, const SIMDint64 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return _mm512_div_epi64(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epi64(a.value, b.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Scaling of a vector of 64-bit unsigned integral complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side complex values to be scaled.
// \param b The right-hand side scalars.
// \return The result of the division.
//
// This operation is only available for AVX2, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcuint64 operator/(const SIMDcuint64 &a, const SIMDuint64 &b) noexcept
#if METRIC_NUMERIC_SVML_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return _mm512_div_epu64(a.value, b.value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_div_epu64(a.value, b.value);
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
/*!\brief Division of two vectors of single precision floating point SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the division.
//
// This operation is only available for SSE, AVX, MIC, and AVX-512.
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat operator/(const SIMDf32<T1> &a, const SIMDf32<T2> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_div_ps((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_div_ps((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_div_ps((*a).eval().value, (*b).eval().value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Scaling of a vector of single precision floating point values complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side complex values to be scaled.
// \param b The right-hand side scalars.
// \return The result of the division.
//
// This operation is only available for SSE, AVX, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcfloat operator/(const SIMDcfloat &a, const SIMDfloat &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_div_ps(a.value, b.value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_div_ps(a.value, b.value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_div_ps(a.value, b.value);
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
/*!\brief Division of two vectors of double precision floating point SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the division.
//
// This operation is only available for SSE, AVX, MIC, and AVX-512.
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble operator/(const SIMDf64<T1> &a, const SIMDf64<T2> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_div_pd((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_div_pd((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_div_pd((*a).eval().value, (*b).eval().value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Scaling of a vector of double precision floating point values complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side complex values to be scaled.
// \param b The right-hand side scalars.
// \return The result of the division.
//
// This operation is only available for SSE, AVX, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcdouble operator/(const SIMDcdouble &a, const SIMDdouble &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_div_pd(a.value, b.value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_div_pd(a.value, b.value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_div_pd(a.value, b.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
