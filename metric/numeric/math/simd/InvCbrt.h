// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_INVCBRT_H
#define METRIC_NUMERIC_MATH_SIMD_INVCBRT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

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
/*!\brief Computes the inverse cubic root for a vector of single precision floating point values.
// \ingroup simd
//
// \param a The vector of single precision floating point values.
// \return The resulting vector.
//
// This operation is only available via the SVML for SSE, AVX, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat invcbrt(const SIMDf32<T> &a) noexcept
#if METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX512F_MODE
{
	return _mm512_recip_ps(_mm512_cbrt_ps((*a).eval().value));
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX_MODE
{
	return _mm256_invcbrt_ps((*a).eval().value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_SSE_MODE
{
	return _mm_invcbrt_ps((*a).eval().value);
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
/*!\brief Computes the inverse cubic root for a vector of double precision floating point values.
// \ingroup simd
//
// \param a The vector of double precision floating point values.
// \return The resulting vector.
//
// This operation is only available via the SVML for SSE, AVX, and AVX-512.
*/
template <typename T> // Type of the operand
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble invcbrt(const SIMDf64<T> &a) noexcept
#if METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX512F_MODE
{
	return _mm512_recip_pd(_mm512_cbrt_pd((*a).eval().value));
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_AVX_MODE
{
	return _mm256_invcbrt_pd((*a).eval().value);
}
#elif METRIC_NUMERIC_SVML_MODE && METRIC_NUMERIC_SSE_MODE
{
	return _mm_invcbrt_pd((*a).eval().value);
}
#else
	= delete;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
