// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_ATAN2_H
#define METRIC_NUMERIC_MATH_SIMD_ATAN2_H
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
/*!\brief Multi-valued inverse tangent of a vector of single precision floating point values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The resulting vector.
//
// This operation is only available via the SVML or SLEEF for SSE, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat atan2(const SIMDf32<T> &a, const SIMDf32<T> &b) noexcept
#if METRIC_NUMERIC_SVML_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_atan2_ps((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_atan2_ps((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_atan2_ps((*a).eval().value, (*b).eval().value);
}
#endif
#elif METRIC_NUMERIC_SLEEF_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return Sleef_atan2f16_u10((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return Sleef_atan2f8_u10((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return Sleef_atan2f4_u10((*a).eval().value, (*b).eval().value);
}
#endif
#elif METRIC_NUMERIC_XSIMD_MODE
{
	return SIMDfloat{xsimd::atan2(xsimd::batch<float>{(*a).eval().value}, xsimd::batch<float>{(*b).eval().value}).data};
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
/*!\brief Multi-valued inverse tangent of a vector of double precision floating point values.
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The resulting vector.
//
// This operation is only available via the SVML or SLEEF for SSE, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble atan2(const SIMDf64<T> &a, const SIMDf64<T> &b) noexcept
#if METRIC_NUMERIC_SVML_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_atan2_pd((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_atan2_pd((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_atan2_pd((*a).eval().value, (*b).eval().value);
}
#endif
#elif METRIC_NUMERIC_SLEEF_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return Sleef_atan2d8_u10((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return Sleef_atan2d4_u10((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return Sleef_atan2d2_u10((*a).eval().value, (*b).eval().value);
}
#endif
#elif METRIC_NUMERIC_XSIMD_MODE
{
	return SIMDdouble{
		xsimd::atan2(xsimd::batch<double>{(*a).eval().value}, xsimd::batch<double>{(*b).eval().value}).data};
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
