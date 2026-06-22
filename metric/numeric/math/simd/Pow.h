// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_POW_H
#define METRIC_NUMERIC_MATH_SIMD_POW_H
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
/*!\brief Computes the exponential value of a vector of single precision floating point values.
// \ingroup simd
//
// \param a The vector of single precision floating point base values.
// \param b The vector of single precision floating point exponents.
// \return The resulting vector.
//
// This operation is only available via the SVML or SLEEF for SSE, AVX, MIC, and AVX-512.
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat pow(const SIMDf32<T1> &a, const SIMDf32<T2> &b) noexcept
#if METRIC_NUMERIC_SVML_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_pow_ps((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_pow_ps((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_pow_ps((*a).eval().value, (*b).eval().value);
}
#endif
#elif METRIC_NUMERIC_SLEEF_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return Sleef_powf16_u10((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return Sleef_powf8_u10((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return Sleef_powf4_u10((*a).eval().value, (*b).eval().value);
}
#endif
#elif METRIC_NUMERIC_XSIMD_MODE
{
	return SIMDfloat{xsimd::pow(xsimd::batch<float>{(*a).eval().value}, xsimd::batch<float>{(*b).eval().value}).data};
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
/*!\brief Computes the exponential value of a vector of double precision floating point values.
// \ingroup simd
//
// \param a The vector of double precision floating point base values.
// \param b The vector of double precision floating point exponents.
// \return The resulting vector.
//
// This operation is only available via the SVML or SLEEF for SSE, AVX, MIC, and AVX-512.
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble pow(const SIMDf64<T1> &a, const SIMDf64<T2> &b) noexcept
#if METRIC_NUMERIC_SVML_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return _mm512_pow_pd((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_pow_pd((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return _mm_pow_pd((*a).eval().value, (*b).eval().value);
}
#endif
#elif METRIC_NUMERIC_SLEEF_MODE
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
{
	return Sleef_powd8_u10((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_AVX_MODE
{
	return Sleef_powd4_u10((*a).eval().value, (*b).eval().value);
}
#elif METRIC_NUMERIC_SSE_MODE
{
	return Sleef_powd2_u10((*a).eval().value, (*b).eval().value);
}
#endif
#elif METRIC_NUMERIC_XSIMD_MODE
{
	return SIMDdouble{
		xsimd::pow(xsimd::batch<double>{(*a).eval().value}, xsimd::batch<double>{(*b).eval().value}).data};
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
