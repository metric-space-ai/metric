// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_SHIFTRV_H
#define METRIC_NUMERIC_MATH_SIMD_SHIFTRV_H
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
/*!\brief Elementwise right-shift of a vector of 16-bit signed integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD vector of 16-bit signed integral values to be shifted.
// \param b The right-hand side SIMD vector of bits to shift.
// \return The result of the right-shift.
//
// This operation is only available for AVX-512.
*/
template <typename T> // Type of both operands
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint16 operator>>(const SIMDint16 &a, const SIMDi16<T> &b) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_srav_epi16(a.value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Elementwise right-shift of a vector of 16-bit unsigned integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD vector of 16-bit unsigned integral values to be shifted.
// \param b The right-hand side SIMD vector of bits to shift.
// \return The result of the right-shift.
//
// This operation is only available for AVX-512.
*/
template <typename T> // Type of both operands
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint16 operator>>(const SIMDuint16 &a, const SIMDi16<T> &b) noexcept
#if METRIC_NUMERIC_AVX512BW_MODE
{
	return _mm512_srlv_epi16(a.value, (*b).value);
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
/*!\brief Elementwise right-shift of a vector of 32-bit signed integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD vector of 32-bit signed integral values to be shifted.
// \param b The right-hand side SIMD vector of bits to shift.
// \return The result of the right-shift.
//
// This operation is only available for AVX2, MIC, and AVX-512.
*/
template <typename T> // Type of both operands
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint32 operator>>(const SIMDint32 &a, const SIMDi32<T> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_srav_epi32(a.value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_srav_epi32(a.value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Elementwise right-shift of a vector of 32-bit unsigned integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD vector of 32-bit unsigned integral values to be shifted.
// \param b The right-hand side SIMD vector of bits to shift.
// \return The result of the right-shift.
//
// This operation is only available for AVX2, MIC, and AVX-512.
*/
template <typename T> // Type of both operands
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint32 operator>>(const SIMDuint32 &a, const SIMDi32<T> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_srlv_epi32(a.value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_srlv_epi32(a.value, (*b).value);
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
/*!\brief Elementwise right-shift of a vector of 64-bit signed integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD vector of 64-bit signed integral values to be shifted.
// \param b The right-hand side SIMD vector of bits to shift.
// \return The result of the right-shift.
//
// This operation is only available for AVX-512.
*/
template <typename T> // Type of both operands
METRIC_NUMERIC_ALWAYS_INLINE const SIMDint64 operator>>(const SIMDint64 &a, const SIMDi64<T> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE
{
	return _mm512_srav_epi64(a.value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Elementwise right-shift of a vector of 64-bit unsigned integral SIMD values.
// \ingroup simd
//
// \param a The left-hand side SIMD vector of 64-bit unsigned integral values to be shifted.
// \param b The right-hand side SIMD vector of bits to shift.
// \return The result of the right-shift.
//
// This operation is only available for AVX2 and AVX-512.
*/
template <typename T> // Type of both operands
METRIC_NUMERIC_ALWAYS_INLINE const SIMDuint64 operator>>(const SIMDuint64 &a, const SIMDi64<T> &b) noexcept
#if METRIC_NUMERIC_AVX512F_MODE
{
	return _mm512_srlv_epi64(a.value, (*b).value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return _mm256_srlv_epi64(a.value, (*b).value);
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
