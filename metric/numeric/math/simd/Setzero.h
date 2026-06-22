// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_SETZERO_H
#define METRIC_NUMERIC_MATH_SIMD_SETZERO_H
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
/*!\brief Setting an integral SIMD type with 8-bit data values to zero.
// \ingroup simd
//
// \param value The value to be set to zero.
// \return void
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE void setzero(SIMDi8<T> &value) noexcept
{
#if METRIC_NUMERIC_AVX512BW_MODE
	(*value).value = _mm512_setzero_si512();
#elif METRIC_NUMERIC_AVX2_MODE
	(*value).value = _mm256_setzero_si256();
#elif METRIC_NUMERIC_SSE2_MODE
	(*value).value = _mm_setzero_si128();
#else
	(*value).value = 0;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Setting an integral SIMD type with 8-bit complex values to zero.
// \ingroup simd
//
// \param value The value to be set to zero.
// \return void
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE void setzero(SIMDci8<T> &value) noexcept
{
#if METRIC_NUMERIC_AVX512BW_MODE
	(*value).value = _mm512_setzero_si512();
#elif METRIC_NUMERIC_AVX2_MODE
	(*value).value = _mm256_setzero_si256();
#elif METRIC_NUMERIC_SSE2_MODE
	(*value).value = _mm_setzero_si128();
#else
	(*value).value = 0;
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  16-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Setting an integral SIMD type with 16-bit data values to zero.
// \ingroup simd
//
// \param value The value to be set to zero.
// \return void
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE void setzero(SIMDi16<T> &value) noexcept
{
#if METRIC_NUMERIC_AVX512BW_MODE
	(*value).value = _mm512_setzero_si512();
#elif METRIC_NUMERIC_AVX2_MODE
	(*value).value = _mm256_setzero_si256();
#elif METRIC_NUMERIC_SSE2_MODE
	(*value).value = _mm_setzero_si128();
#else
	(*value).value = 0;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Setting an integral SIMD type with 16-bit complex values to zero.
// \ingroup simd
//
// \param value The value to be set to zero.
// \return void
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE void setzero(SIMDci16<T> &value) noexcept
{
#if METRIC_NUMERIC_AVX512BW_MODE
	(*value).value = _mm512_setzero_si512();
#elif METRIC_NUMERIC_AVX2_MODE
	(*value).value = _mm256_setzero_si256();
#elif METRIC_NUMERIC_SSE2_MODE
	(*value).value = _mm_setzero_si128();
#else
	(*value).value = 0;
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  32-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Setting an integral SIMD type with 32-bit data values to zero.
// \ingroup simd
//
// \param value The value to be set to zero.
// \return void
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE void setzero(SIMDi32<T> &value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	(*value).value = _mm512_setzero_si512();
#elif METRIC_NUMERIC_AVX2_MODE
	(*value).value = _mm256_setzero_si256();
#elif METRIC_NUMERIC_SSE2_MODE
	(*value).value = _mm_setzero_si128();
#else
	(*value).value = 0;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Setting an integral SIMD type with 32-bit complex values to zero.
// \ingroup simd
//
// \param value The value to be set to zero.
// \return void
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE void setzero(SIMDci32<T> &value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	(*value).value = _mm512_setzero_si512();
#elif METRIC_NUMERIC_AVX2_MODE
	(*value).value = _mm256_setzero_si256();
#elif METRIC_NUMERIC_SSE2_MODE
	(*value).value = _mm_setzero_si128();
#else
	(*value).value = 0;
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  64-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Setting an integral SIMD type with 64-bit data values to zero.
// \ingroup simd
//
// \param value The value to be set to zero.
// \return void
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE void setzero(SIMDi64<T> &value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	(*value).value = _mm512_setzero_si512();
#elif METRIC_NUMERIC_AVX2_MODE
	(*value).value = _mm256_setzero_si256();
#elif METRIC_NUMERIC_SSE2_MODE
	(*value).value = _mm_setzero_si128();
#else
	(*value).value = 0;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Setting an integral SIMD type with 64-bit complex values to zero.
// \ingroup simd
//
// \param value The value to be set to zero.
// \return void
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE void setzero(SIMDci64<T> &value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	(*value).value = _mm512_setzero_si512();
#elif METRIC_NUMERIC_AVX2_MODE
	(*value).value = _mm256_setzero_si256();
#elif METRIC_NUMERIC_SSE2_MODE
	(*value).value = _mm_setzero_si128();
#else
	(*value).value = 0;
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  32-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Setting a floating point SIMD type with 32-bit single precision data values to zero.
// \ingroup simd
//
// \param value The value to be set to zero.
// \return void
*/
METRIC_NUMERIC_ALWAYS_INLINE void setzero(SIMDfloat &value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	value.value = _mm512_setzero_ps();
#elif METRIC_NUMERIC_AVX_MODE
	value.value = _mm256_setzero_ps();
#elif METRIC_NUMERIC_SSE_MODE
	value.value = _mm_setzero_ps();
#else
	value.value = 0.0F;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Setting a floating point SIMD type with 32-bit single precision complex values to zero.
// \ingroup simd
//
// \param value The value to be set to zero.
// \return void
*/
METRIC_NUMERIC_ALWAYS_INLINE void setzero(SIMDcfloat &value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	value.value = _mm512_setzero_ps();
#elif METRIC_NUMERIC_AVX_MODE
	value.value = _mm256_setzero_ps();
#elif METRIC_NUMERIC_SSE_MODE
	value.value = _mm_setzero_ps();
#else
	value.value = 0.0F;
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  64-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Setting a floating point SIMD type with 64-bit double precision data values to zero.
// \ingroup simd
//
// \param value The value to be set to zero.
// \return void
*/
METRIC_NUMERIC_ALWAYS_INLINE void setzero(SIMDdouble &value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	value.value = _mm512_setzero_pd();
#elif METRIC_NUMERIC_AVX_MODE
	value.value = _mm256_setzero_pd();
#elif METRIC_NUMERIC_SSE2_MODE
	value.value = _mm_setzero_pd();
#else
	value.value = 0.0;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Setting a floating point SIMD type with 32-bit double precision complex values to zero.
// \ingroup simd
//
// \param value The value to be set to zero.
// \return void
*/
METRIC_NUMERIC_ALWAYS_INLINE void setzero(SIMDcdouble &value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	value.value = _mm512_setzero_pd();
#elif METRIC_NUMERIC_AVX_MODE
	value.value = _mm256_setzero_pd();
#elif METRIC_NUMERIC_SSE2_MODE
	value.value = _mm_setzero_pd();
#else
	value.value = 0.0;
#endif
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
