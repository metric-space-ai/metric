// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_FLOOR_H
#define METRIC_NUMERIC_MATH_SIMD_FLOOR_H
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
/*!\brief Computes the largest integer value not greater than the given value for a vector of
//        single precision floating point values.
// \ingroup simd
//
// \param a The vector of single precision floating point values.
// \return The resulting vector.
//
// This operation is only available via the SVML or SLEEF for SSE, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat floor(const SIMDf32<T> &a) noexcept
#if METRIC_NUMERIC_SVML_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return _mm512_floor_ps((*a).eval().value);
}
#elif METRIC_NUMERIC_SLEEF_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return Sleef_floorf16((*a).eval().value);
}
#elif METRIC_NUMERIC_XSIMD_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return SIMDfloat{xsimd::floor(xsimd::batch<float>{(*a).eval().value}).data};
}
#elif METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	= delete;
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_floor_ps((*a).eval().value);
}
#elif METRIC_NUMERIC_SSE4_MODE
{
	return _mm_floor_ps((*a).eval().value);
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
/*!\brief Computes the largest integer value not greater than the given value for a vector of
//        double precision floating point values.
// \ingroup simd
//
// \param a The vector of double precision floating point values.
// \return The resulting vector.
//
// This operation is only available via the SVML or SLEEF for SSE, AVX, MIC, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble floor(const SIMDf64<T> &a) noexcept
#if METRIC_NUMERIC_SVML_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return _mm512_floor_pd((*a).eval().value);
}
#elif METRIC_NUMERIC_SLEEF_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return Sleef_floord8((*a).eval().value);
}
#elif METRIC_NUMERIC_XSIMD_MODE && (METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE)
{
	return SIMDdouble{xsimd::floor(xsimd::batch<double>{(*a).eval().value}).data};
}
#elif METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	= delete;
#elif METRIC_NUMERIC_AVX_MODE
{
	return _mm256_floor_pd((*a).eval().value);
}
#elif METRIC_NUMERIC_SSE4_MODE
{
	return _mm_floor_pd((*a).eval().value);
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
