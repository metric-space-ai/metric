// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_SET_H
#define METRIC_NUMERIC_MATH_SIMD_SET_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/simd/BasicTypes.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/system/Vectorization.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/StaticAssert.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/constraints/Integral.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/HasSize.h>
#include <metric/numeric/util/typetraits/IsIntegral.h>
#include <metric/numeric/util/typetraits/IsSigned.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  8-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Sets all values in the vector to the given 1-byte integral value.
// \ingroup simd
//
// \param value The given 1-byte integral value.
// \return The set vector of 1-byte integral values.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 1UL>, If_t<IsSigned_v<T>, SIMDint8, SIMDuint8>> set(T value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE
	return _mm512_set1_epi8(value);
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_set1_epi8(value);
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_set1_epi8(value);
#else
	return value;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Sets all values in the vector to the given 1-byte integral complex value.
// \ingroup simd
//
// \param value The given 1-byte integral complex value.
// \return The set vector of 1-byte integral complex values.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 1UL>, If_t<IsSigned_v<T>, SIMDcint8, SIMDcuint8>>
	set(complex<T> value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE
	return _mm512_set1_epi16(reinterpret_cast<const int16_t &>(value));
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_set_epi8(
		value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
		value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
		value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
		value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real());
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_set_epi8(value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
						value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
						value.imag(), value.real(), value.imag(), value.real());
#else
	return value;
#endif
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<T>) == 2UL * sizeof(T));
}
//*************************************************************************************************

//=================================================================================================
//
//  16-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Sets all values in the vector to the given 2-byte integral value.
// \ingroup simd
//
// \param value The given 2-byte integral value.
// \return The set vector of 2-byte integral values.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 2UL>, If_t<IsSigned_v<T>, SIMDint16, SIMDuint16>> set(T value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE
	return _mm512_set1_epi16(value);
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_set1_epi16(value);
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_set1_epi16(value);
#else
	return value;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Sets all values in the vector to the given 2-byte integral complex value.
// \ingroup simd
//
// \param value The given 2-byte integral complex value.
// \return The set vector of 2-byte integral complex values.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 2UL>, If_t<IsSigned_v<T>, SIMDcint16, SIMDcuint16>>
	set(complex<T> value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE
	return _mm512_set1_epi32(reinterpret_cast<const int32_t &>(value));
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_set_epi16(value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
							value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
							value.imag(), value.real(), value.imag(), value.real());
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_set_epi16(value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
						 value.imag(), value.real());
#else
	return value;
#endif
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<T>) == 2UL * sizeof(T));
}
//*************************************************************************************************

//=================================================================================================
//
//  32-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Sets all values in the vector to the given 4-byte integral value.
// \ingroup simd
//
// \param value The given 4-byte integral value.
// \return The set vector of 4-byte integral values.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 4UL>, If_t<IsSigned_v<T>, SIMDint32, SIMDuint32>> set(T value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return _mm512_set1_epi32(value);
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_set1_epi32(value);
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_set1_epi32(value);
#else
	return value;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Sets all values in the vector to the given 4-byte integral complex value.
// \ingroup simd
//
// \param value The given 4-byte integral complex value.
// \return The set vector of 4-byte integral complex values.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 4UL>, If_t<IsSigned_v<T>, SIMDcint32, SIMDcuint32>>
	set(complex<T> value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return _mm512_set_epi32(value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
							value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
							value.imag(), value.real(), value.imag(), value.real());
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_set_epi32(value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
							value.imag(), value.real());
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_set_epi32(value.imag(), value.real(), value.imag(), value.real());
#else
	return value;
#endif
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<T>) == 2UL * sizeof(T));
}
//*************************************************************************************************

//=================================================================================================
//
//  64-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Sets all values in the vector to the given 8-byte integral value.
// \ingroup simd
//
// \param value The given 8-byte integral value.
// \return The set vector of 8-byte integral values.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 8UL>, If_t<IsSigned_v<T>, SIMDint64, SIMDuint64>> set(T value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return _mm512_set1_epi64(value);
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_set1_epi64x(value);
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_set1_epi64x(value);
#else
	return value;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Sets all values in the vector to the given 8-byte integral complex value.
// \ingroup simd
//
// \param value The given 8-byte integral complex value.
// \return The set vector of 8-byte integral complex values.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 8UL>, If_t<IsSigned_v<T>, SIMDcint64, SIMDcuint64>>
	set(complex<T> value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return _mm512_set_epi64(value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
							value.imag(), value.real());
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_set_epi64x(value.imag(), value.real(), value.imag(), value.real());
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_set_epi64x(value.imag(), value.real());
#else
	return value;
#endif
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<T>) == 2UL * sizeof(T));
}
//*************************************************************************************************

//=================================================================================================
//
//  32-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Sets all values in the vector to the given \c float value.
// \ingroup simd
//
// \param value The given \c float value.
// \return The set vector of \c float values.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat set(float value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return _mm512_set1_ps(value);
#elif METRIC_NUMERIC_AVX_MODE
	return _mm256_set1_ps(value);
#elif METRIC_NUMERIC_SSE_MODE
	return _mm_set1_ps(value);
#else
	return value;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Sets all values in the vector to the given \c complex<float> value.
// \ingroup simd
//
// \param value The given \c complex<float> value.
// \return The set vector of \c complex<float> values.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcfloat set(const complex<float> &value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return _mm512_set_ps(value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
						 value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
						 value.imag(), value.real(), value.imag(), value.real());
#elif METRIC_NUMERIC_AVX_MODE
	return _mm256_set_ps(value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
						 value.imag(), value.real());
#elif METRIC_NUMERIC_SSE_MODE
	return _mm_set_ps(value.imag(), value.real(), value.imag(), value.real());
#else
	return value;
#endif
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<float>) == 2UL * sizeof(float));
}
//*************************************************************************************************

//=================================================================================================
//
//  64-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Sets all values in the vector to the given \c double value.
// \ingroup simd
//
// \param value The given \c double value.
// \return The set vector of \c double values.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble set(double value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return _mm512_set1_pd(value);
#elif METRIC_NUMERIC_AVX_MODE
	return _mm256_set1_pd(value);
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_set1_pd(value);
#else
	return value;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Sets all values in the vector to the given \c complex<double> value.
// \ingroup simd
//
// \param value The given \c complex<double> value.
// \return The set vector of \c complex<double> values.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcdouble set(const complex<double> &value) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return _mm512_set_pd(value.imag(), value.real(), value.imag(), value.real(), value.imag(), value.real(),
						 value.imag(), value.real());
#elif METRIC_NUMERIC_AVX_MODE
	return _mm256_set_pd(value.imag(), value.real(), value.imag(), value.real());
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_set_pd(value.imag(), value.real());
#else
	return value;
#endif
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<double>) == 2UL * sizeof(double));
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
