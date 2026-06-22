// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_ADD_H
#define METRIC_NUMERIC_MATH_SIMD_ADD_H
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
/*!\brief Addition of two vectors of 8-bit integral SIMD values of the same type.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE2, AVX2 and AVX512BW.
*/
template <typename T> // Type of both operands
METRIC_NUMERIC_ALWAYS_INLINE const T operator+(const SIMDi8<T> &a, const SIMDi8<T> &b) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_add_epi8((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_add_epi8((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_add_epi8((*a).value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition of two vectors of 8-bit integral SIMD values of different type.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE2, AVX2 and AVX512BW.
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint8 operator+(const SIMDi8<T1> &a, const SIMDi8<T2> &b) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_add_epi8((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_add_epi8((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_add_epi8((*a).value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition of two vectors of 8-bit integral complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE2, AVX2 and AVX512BW.
*/
template <typename T> // Type of both operands
METRIC_NUMERIC_ALWAYS_INLINE const T operator+(const SIMDci8<T> &a, const SIMDci8<T> &b) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_add_epi8((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_add_epi8((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_add_epi8((*a).value, (*b).value);
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
/*!\brief Addition of two vectors of 16-bit integral SIMD values of the same type.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE2, AVX2 and AVX512BW.
*/
template <typename T> // Type of both operands
METRIC_NUMERIC_ALWAYS_INLINE const T operator+(const SIMDi16<T> &a, const SIMDi16<T> &b) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_add_epi16((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_add_epi16((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_add_epi16((*a).value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition of two vectors of 16-bit integral SIMD values of different type.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE2, AVX2 and AVX512BW.
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint16 operator+(const SIMDi16<T1> &a, const SIMDi16<T2> &b) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_add_epi16((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_add_epi16((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_add_epi16((*a).value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition of two vectors of 16-bit integral complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE2, AVX2 and AVX512BW.
*/
template <typename T> // Type of both operands
METRIC_NUMERIC_ALWAYS_INLINE const T operator+(const SIMDci16<T> &a, const SIMDci16<T> &b) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_add_epi16((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_add_epi16((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_add_epi16((*a).value, (*b).value);
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
/*!\brief Addition of two vectors of 32-bit integral SIMD values of the same type.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE2, AVX2, MIC, and AVX-512.
*/
template <typename T> // Type of both operands
METRIC_NUMERIC_ALWAYS_INLINE const T operator+(const SIMDi32<T> &a, const SIMDi32<T> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_add_epi32((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_add_epi32((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_add_epi32((*a).value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition of two vectors of 32-bit integral SIMD values of different type.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE2, AVX2, MIC, and AVX-512.
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint32 operator+(const SIMDi32<T1> &a, const SIMDi32<T2> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_add_epi32((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_add_epi32((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_add_epi32((*a).value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition of two vectors of 32-bit integral complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE2, AVX2, MIC, and AVX-512.
*/
template <typename T> // Type of both operands
METRIC_NUMERIC_ALWAYS_INLINE const T operator+(const SIMDci32<T> &a, const SIMDci32<T> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_add_epi32((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_add_epi32((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_add_epi32((*a).value, (*b).value);
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
/*!\brief Addition of two vectors of 64-bit integral SIMD values of the same type.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE2, AVX2, MIC, and AVX-512.
*/
template <typename T> // Type of the left-hand side operand
METRIC_NUMERIC_ALWAYS_INLINE const T operator+(const SIMDi64<T> &a, const SIMDi64<T> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_add_epi64((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_add_epi64((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_add_epi64((*a).value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition of two vectors of 64-bit integral SIMD values of different type.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE2, AVX2, MIC, and AVX-512.
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint64 operator+(const SIMDi64<T1> &a, const SIMDi64<T2> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_add_epi64((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_add_epi64((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_add_epi64((*a).value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition of two vectors of 64-bit integral complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE2, AVX2, MIC, and AVX-512.
*/
template <typename T> // Type of both operands
METRIC_NUMERIC_ALWAYS_INLINE const T operator+(const SIMDci64<T> &a, const SIMDci64<T> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_add_epi64((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_add_epi64((*a).value, (*b).value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_add_epi64((*a).value, (*b).value);
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
/*!\brief Addition of two vectors of single precision floating point SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE, AVX, MIC, and AVX-512.
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat operator+(const SIMDf32<T1> &a, const SIMDf32<T2> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_add_ps((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_add_ps((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_add_ps((*a).eval().value, (*b).eval().value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition of two vectors of single precision complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE, AVX, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcfloat operator+(const SIMDcfloat &a, const SIMDcfloat &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_add_ps(a.value, b.value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_add_ps(a.value, b.value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_add_ps(a.value, b.value);
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
/*!\brief Addition of two vectors of double precision floating point SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE2, AVX, MIC, and AVX-512.
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble operator+(const SIMDf64<T1> &a, const SIMDf64<T2> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_add_pd((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_add_pd((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_add_pd((*a).eval().value, (*b).eval().value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition of two vectors of double precision complex SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The result of the addition.
//
// This operation is only available for SSE2, AVX, MIC, and AVX-512.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcdouble operator+(const SIMDcdouble &a, const SIMDcdouble &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_add_pd(a.value, b.value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_add_pd(a.value, b.value);
}
#elif METRIC_NUMERIC_SSE2_MODE
{
	return _mm_add_pd(a.value, b.value);
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
