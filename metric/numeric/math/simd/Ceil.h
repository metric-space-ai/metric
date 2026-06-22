// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_CEIL_H
#define METRIC_NUMERIC_MATH_SIMD_CEIL_H
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
/*!\brief Computes the largest integer value not less than the given value for a vector of single
//        precision floating point values.
// \ingroup simd
//
// \param a The vector of single precision floating point values.
// \return The resulting vector.
//
// This operation is only available via the SVML or SLEEF for SSE, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat ceil(const SIMDf32<T> &a) noexcept
#if METRIC_NUMERIC_SVML_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return _mm512_ceil_ps((*a).eval().value);
}
#elif METRIC_NUMERIC_SLEEF_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return Sleef_ceilf16((*a).eval().value);
}
#elif METRIC_NUMERIC_XSIMD_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return SIMDfloat{xsimd::ceil(xsimd::batch<float>{(*a).eval().value}).data};
}
#elif METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	= delete;
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_ceil_ps((*a).eval().value);
}
#elif METRIC_NUMERIC_SSE4_MODE
{
	return _mm_ceil_ps((*a).eval().value);
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
/*!\brief Computes the largest integer value not less than the given value for a vector of double
//        precision floating point values.
// \ingroup simd
//
// \param a The vector of double precision floating point values.
// \return The resulting vector.
//
// This operation is only available via the SVML or SLEEF for SSE, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble ceil(const SIMDf64<T> &a) noexcept
#if METRIC_NUMERIC_SVML_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return _mm512_ceil_pd((*a).eval().value);
}
#elif METRIC_NUMERIC_SLEEF_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return Sleef_ceild8((*a).eval().value);
}
#elif METRIC_NUMERIC_XSIMD_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return SIMDdouble{xsimd::ceil(xsimd::batch<double>{(*a).eval().value}).data};
}
#elif METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	= delete;
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_ceil_pd((*a).eval().value);
}
#elif METRIC_NUMERIC_SSE4_MODE
{
	return _mm_ceil_pd((*a).eval().value);
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
