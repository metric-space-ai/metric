// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_HYPOT_H
#define METRIC_NUMERIC_MATH_SIMD_HYPOT_H
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
/*!\brief Computes the length of the hypotenous of a right triangle of a vector of single
//        precision floating point values (\f$ \sqrt( a^2 + b^2 ) \f$).
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The resulting vector.
//
// This function computes the square root of the sum of the squares of \a a and \a b, which
// corresponds to the length of the hypotenous of a right triangle. This operation is only
// available via the SVML or SLEEF or Sleef for SSE, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat hypot(const SIMDf32<T> &a, const SIMDf32<T> &b) noexcept
#if METRIC_NUMERIC_SVML_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_hypot_ps((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_hypot_ps((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_hypot_ps((*a).eval().value, (*b).eval().value);
}
#endif
#elif METRIC_NUMERIC_SLEEF_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return Sleef_hypotf16_u05avx512f((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return Sleef_hypotf8_u05avx2((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return Sleef_hypotf8_u05avx((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return Sleef_hypotf4_u05((*a).eval().value, (*b).eval().value);
}
#endif
#elif METRIC_NUMERIC_XSIMD_MODE
{
	return SIMDfloat{xsimd::hypot(xsimd::batch<float>{(*a).eval().value}, xsimd::batch<float>{(*b).eval().value}).data};
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
/*!\brief Computes the length of the hypotenous of a right triangle of a vector of double
//        precision floating point values (\f$ \sqrt( a^2 + b^2 ) \f$).
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The resulting vector.
//
// This function computes the square root of the sum of the squares of \a a and \a b, which
// corresponds to the length of the hypotenous of a right triangle. This operation is only
// available via the SVML or SLEEF or Sleef for SSE, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble hypot(const SIMDf64<T> &a, const SIMDf64<T> &b) noexcept
#if METRIC_NUMERIC_SVML_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_hypot_pd((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_hypot_pd((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_hypot_pd((*a).eval().value, (*b).eval().value);
}
#endif
#elif METRIC_NUMERIC_SLEEF_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return Sleef_hypotd8_u05avx512f((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX2_MODE
{
	return Sleef_hypotd4_u05avx2((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return Sleef_hypotd4_u05avx((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return Sleef_hypotd2_u05((*a).eval().value, (*b).eval().value);
}
#endif
#elif METRIC_NUMERIC_XSIMD_MODE
{
	return SIMDdouble{
		xsimd::hypot(xsimd::batch<double>{(*a).eval().value}, xsimd::batch<double>{(*b).eval().value}).data};
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
