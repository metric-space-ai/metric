// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_CONJ_H
#define METRIC_NUMERIC_MATH_SIMD_CONJ_H
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
/*!\brief Complex conjugate of a vector of 8-bit integral values.
// \ingroup simd
//
// \param a The vector of 8-bit integral values.
// \return The complex conjugate values.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const T conj(const SIMDi8<T> &a) noexcept
{
	return a;
}
//*************************************************************************************************

//=================================================================================================
//
//  16-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Complex conjugate of a vector of 16-bit integral values.
// \ingroup simd
//
// \param a The vector of 16-bit integral values.
// \return The complex conjugate values.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const T conj(const SIMDi16<T> &a) noexcept
{
	return a;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Complex conjugate of a vector of 16-bit integral complex values.
// \ingroup simd
//
// \param a The vector of 16-bit integral complex values.
// \return The complex conjugate values.
//
// This operation is only available for SSE2, AVX2, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcint16 conj(const SIMDcint16 &a) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_mask_sub_epi16(a.value, 0XAAAAAAAA, _mm512_setzero_si512(), a.value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_mullo_epi16(a.value, _mm256_set_epi16(-1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1));
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_mullo_epi16(a.value, _mm_set_epi16(-1, 1, -1, 1, -1, 1, -1, 1));
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
/*!\brief Complex conjugate of a vector of 32-bit integral values.
// \ingroup simd
//
// \param a The vector of 32-bit integral values.
// \return The complex conjugate values.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const T conj(const SIMDi32<T> &a) noexcept
{
	return a;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Complex conjugate of a vector of 32-bit integral complex values.
// \ingroup simd
//
// \param a The vector of 32-bit integral complex values.
// \return The complex conjugate values.
//
// This operation is only available for SSE4, AVX2, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcint32 conj(const SIMDcint32 &a) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_mullo_epi32(a.value, _mm512_set_epi32(-1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1));
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_mullo_epi32(a.value, _mm256_set_epi32(-1, 1, -1, 1, -1, 1, -1, 1));
}
#elif METRIC_NUMERIC_SSE4_MODE
{
	return _mm_mullo_epi32(a.value, _mm_set_epi32(-1, 1, -1, 1));
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
/*!\brief Complex conjugate of a vector of 64-bit integral values.
// \ingroup simd
//
// \param a The vector of 64-bit integral values.
// \return The complex conjugate values.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const T conj(const SIMDi64<T> &a) noexcept
{
	return a;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Complex conjugate of a vector of 64-bit integral complex values.
// \ingroup simd
//
// \param a The vector of 64-bit integral values.
// \return The complex conjugate values.
//
// This operation is only available for AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcint64 conj(const SIMDcint64 &a) noexcept
#if METRIC_NUMERIC_AVX512F_MODE
{
	return _mm512_mask_sub_epi64(a.value, 0XAA, _mm512_setzero_si512(), a.value);
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
/*!\brief Complex conjugate of a vector of single precision floating point values.
// \ingroup simd
//
// \param a The vector of single precision floating point values.
// \return The complex conjugate values.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const T conj(const SIMDf32<T> &a) noexcept
{
	return (*a);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Complex conjugate of a vector of single precision complex values.
// \ingroup simd
//
// \param a The vector of single precision complex values.
// \return The complex conjugate values.
//
// This operation is only available for SSE2, AVX, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcfloat conj(const SIMDcfloat &a) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_mask_sub_ps(a.value, 0XAAAA, _mm512_setzero_ps(), a.value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_mul_ps(a.value, _mm256_set_ps(-1.0F, 1.0F, -1.0F, 1.0F, -1.0F, 1.0F, -1.0F, 1.0F));
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_mul_ps(a.value, _mm_set_ps(-1.0F, 1.0F, -1.0F, 1.0F));
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
/*!\brief Complex conjugate of a vector of double precision floating point values.
// \ingroup simd
//
// \param a The vector of double precision floating point values.
// \return The complex conjugate values.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const T conj(const SIMDf64<T> &a) noexcept
{
	return (*a);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Complex conjugate of a vector of double precision complex values.
// \ingroup simd
//
// \param a The vector of double precision complex values.
// \return The complex conjugate values.
//
// This operation is only available for SSE2, AVX, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcdouble conj(const SIMDcdouble &a) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_mask_sub_pd(a.value, 0XAA, _mm512_setzero_pd(), a.value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_mul_pd(a.value, _mm256_set_pd(-1.0, 1.0, -1.0, 1.0));
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_mul_pd(a.value, _mm_set_pd(-1.0, 1.0));
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
