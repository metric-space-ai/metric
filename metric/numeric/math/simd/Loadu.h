// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_LOADU_H
#define METRIC_NUMERIC_MATH_SIMD_LOADU_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/simd/BasicTypes.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/system/Vectorization.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/StaticAssert.h>
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
/*!\brief Loads a vector of 1-byte integral values.
// \ingroup simd
//
// \param address The first integral value to be loaded.
// \return The loaded vector of integral values.
//
// This function loads a vector of 1-byte integral values. In contrast to the according \c loada()
// function, the given address is not required to be properly aligned.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 1UL>, If_t<IsSigned_v<T>, SIMDint8, SIMDuint8>>
	loadu(const T *address) noexcept
{
#if METRIC_NUMERIC_AVX512BW_MODE
	return _mm512_loadu_si512(reinterpret_cast<const __m512i *>(address));
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_loadu_si256(reinterpret_cast<const __m256i *>(address));
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_loadu_si128(reinterpret_cast<const __m128i *>(address));
#else
	return *address;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Loads a vector of 1-byte integral complex values.
// \ingroup simd
//
// \param address The first integral complex value to be loaded.
// \return The loaded vector of integral complex values.
//
// This function loads a vector of 1-byte integral complex values. In contrast to the according
// \c loada() function, the given address is not required to be properly aligned.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 1UL>, If_t<IsSigned_v<T>, SIMDcint8, SIMDcuint8>>
	loadu(const complex<T> *address) noexcept
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<T>) == 2UL * sizeof(T));

#if METRIC_NUMERIC_AVX512BW_MODE
	return _mm512_loadu_si512(reinterpret_cast<const __m512i *>(address));
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_loadu_si256(reinterpret_cast<const __m256i *>(address));
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_loadu_si128(reinterpret_cast<const __m128i *>(address));
#else
	return If_t<IsSigned_v<T>, SIMDcint8, SIMDcuint8>(*address);
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  16-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Loads a vector of 2-byte integral values.
// \ingroup simd
//
// \param address The first integral value to be loaded.
// \return The loaded vector of integral values.
//
// This function loads a vector of 2-byte integral values. In contrast to the according \c loada()
// function, the given address is not required to be properly aligned.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 2UL>, If_t<IsSigned_v<T>, SIMDint16, SIMDuint16>>
	loadu(const T *address) noexcept
{
#if METRIC_NUMERIC_AVX512BW_MODE
	return _mm512_loadu_si512(reinterpret_cast<const __m512i *>(address));
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_loadu_si256(reinterpret_cast<const __m256i *>(address));
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_loadu_si128(reinterpret_cast<const __m128i *>(address));
#else
	return *address;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Loads a vector of 2-byte integral complex values.
// \ingroup simd
//
// \param address The first integral complex value to be loaded.
// \return The loaded vector of integral complex values.
//
// This function loads a vector of 2-byte integral complex values. In contrast to the according
// \c loada() function, the given address is not required to be properly aligned.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 2UL>, If_t<IsSigned_v<T>, SIMDcint16, SIMDcuint16>>
	loadu(const complex<T> *address) noexcept
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<T>) == 2UL * sizeof(T));

#if METRIC_NUMERIC_AVX512BW_MODE
	return _mm512_loadu_si512(reinterpret_cast<const __m512i *>(address));
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_loadu_si256(reinterpret_cast<const __m256i *>(address));
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_loadu_si128(reinterpret_cast<const __m128i *>(address));
#else
	return If_t<IsSigned_v<T>, SIMDcint16, SIMDcuint16>(*address);
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  32-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Loads a vector of 4-byte integral values.
// \ingroup simd
//
// \param address The first integral value to be loaded.
// \return The loaded vector of integral values.
//
// This function loads a vector of 4-byte integral values. In contrast to the according \c loada()
// function, the given address is not required to be properly aligned.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 4UL>, If_t<IsSigned_v<T>, SIMDint32, SIMDuint32>>
	loadu(const T *address) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE
	return _mm512_maskz_loadu_epi32(0xFFFF, reinterpret_cast<const __m512i *>(address));
#elif METRIC_NUMERIC_MIC_MODE
	__m512i v1 = _mm512_setzero_si512();
	v1 = _mm512_loadunpacklo_epi32(v1, address);
	v1 = _mm512_loadunpackhi_epi32(v1, address + 16UL);
	return v1;
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_loadu_si256(reinterpret_cast<const __m256i *>(address));
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_loadu_si128(reinterpret_cast<const __m128i *>(address));
#else
	return If_t<IsSigned_v<T>, SIMDint32, SIMDuint32>(*address);
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Loads a vector of 4-byte integral complex values.
// \ingroup simd
//
// \param address The first integral complex value to be loaded.
// \return The loaded vector of integral complex values.
//
// This function loads a vector of 4-byte integral complex values. In contrast to the according
// \c loada() function, the given address is not required to be properly aligned.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 4UL>, If_t<IsSigned_v<T>, SIMDcint32, SIMDcuint32>>
	loadu(const complex<T> *address) noexcept
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<T>) == 2UL * sizeof(T));

#if METRIC_NUMERIC_AVX512F_MODE
	return _mm512_maskz_loadu_epi32(0xFFFF, address);
#elif METRIC_NUMERIC_MIC_MODE
	__m512i v1 = _mm512_setzero_si512();
	v1 = _mm512_loadunpacklo_epi32(v1, address);
	v1 = _mm512_loadunpackhi_epi32(v1, address + 8UL);
	return v1;
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_loadu_si256(reinterpret_cast<const __m256i *>(address));
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_loadu_si128(reinterpret_cast<const __m128i *>(address));
#else
	return *address;
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  64-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Loads a vector of 8-byte integral values.
// \ingroup simd
//
// \param address The first integral value to be loaded.
// \return The loaded vector of integral values.
//
// This function loads a vector of 8-byte integral values. In contrast to the according \c loada()
// function, the given address is not required to be properly aligned.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 8UL>, If_t<IsSigned_v<T>, SIMDint64, SIMDuint64>>
	loadu(const T *address) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE
	return _mm512_maskz_loadu_epi64(0xFF, address);
#elif METRIC_NUMERIC_MIC_MODE
	__m512i v1 = _mm512_setzero_si512();
	v1 = _mm512_loadunpacklo_epi64(v1, address);
	v1 = _mm512_loadunpackhi_epi64(v1, address + 8UL);
	return v1;
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_loadu_si256(reinterpret_cast<const __m256i *>(address));
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_loadu_si128(reinterpret_cast<const __m128i *>(address));
#else
	return *address;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Loads a vector of 8-byte integral complex values.
// \ingroup simd
//
// \param address The first integral complex value to be loaded.
// \return The loaded vector of integral complex values.
//
// This function loads a vector of 8-byte integral complex values. In contrast to the according
// \c loada() function, the given address is not required to be properly aligned.
*/
template <typename T> // Type of the integral value
METRIC_NUMERIC_ALWAYS_INLINE const
	EnableIf_t<IsIntegral_v<T> && HasSize_v<T, 8UL>, If_t<IsSigned_v<T>, SIMDcint64, SIMDcuint64>>
	loadu(const complex<T> *address) noexcept
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<T>) == 2UL * sizeof(T));

#if METRIC_NUMERIC_AVX512F_MODE
	return _mm512_maskz_loadu_epi64(0xFF, address);
#elif METRIC_NUMERIC_MIC_MODE
	__m512i v1 = _mm512_setzero_si512();
	v1 = _mm512_loadunpacklo_epi64(v1, address);
	v1 = _mm512_loadunpackhi_epi64(v1, address + 4UL);
	return v1;
#elif METRIC_NUMERIC_AVX2_MODE
	return _mm256_loadu_si256(reinterpret_cast<const __m256i *>(address));
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_loadu_si128(reinterpret_cast<const __m128i *>(address));
#else
	return If_t<IsSigned_v<T>, SIMDcint64, SIMDcuint64>(*address);
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  32-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Loads a vector of 'float' values.
// \ingroup simd
//
// \param address The first 'float' value to be loaded.
// \return The loaded vector of 'float' values.
//
// This function loads a vector of 'float' values. In contrast to the according \c loada()
// function, the given address is not required to be properly aligned.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDfloat loadu(const float *address) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE
	return _mm512_loadu_ps(address);
#elif METRIC_NUMERIC_MIC_MODE
	__m512 v1 = _mm512_setzero_ps();
	v1 = _mm512_loadunpacklo_ps(v1, address);
	v1 = _mm512_loadunpackhi_ps(v1, address + 16UL);
	return v1;
#elif METRIC_NUMERIC_AVX_MODE
	return _mm256_loadu_ps(address);
#elif METRIC_NUMERIC_SSE_MODE
	return _mm_loadu_ps(address);
#else
	return *address;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Loads a vector of 'complex<float>' values.
// \ingroup simd
//
// \param address The first 'complex<float>' value to be loaded.
// \return The loaded vector of 'complex<float>' values.
//
// This function loads a vector of 'complex<float>' values. In contrast to the according \c loada()
// function, the given address is not required to be properly aligned.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcfloat loadu(const complex<float> *address) noexcept
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<float>) == 2UL * sizeof(float));

#if METRIC_NUMERIC_AVX512F_MODE
	return _mm512_loadu_ps(address);
#elif METRIC_NUMERIC_MIC_MODE
	__m512 v1 = _mm512_setzero_ps();
	v1 = _mm512_loadunpacklo_ps(v1, reinterpret_cast<const float *>(address));
	v1 = _mm512_loadunpackhi_ps(v1, reinterpret_cast<const float *>(address + 8UL));
	return v1;
#elif METRIC_NUMERIC_AVX_MODE
	return _mm256_loadu_ps(reinterpret_cast<const float *>(address));
#elif METRIC_NUMERIC_SSE_MODE
	return _mm_loadu_ps(reinterpret_cast<const float *>(address));
#else
	return *address;
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  64-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Loads a vector of 'double' values.
// \ingroup simd
//
// \param address The first 'double' value to be loaded.
// \return The loaded vector of 'double' values.
//
// This function loads a vector of 'double' values. In contrast to the according \c loada()
// function, the given address is not required to be properly aligned.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDdouble loadu(const double *address) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE
	return _mm512_loadu_pd(address);
#elif METRIC_NUMERIC_MIC_MODE
	__m512d v1 = _mm512_setzero_pd();
	v1 = _mm512_loadunpacklo_pd(v1, address);
	v1 = _mm512_loadunpackhi_pd(v1, address + 8UL);
	return v1;
#elif METRIC_NUMERIC_AVX_MODE
	return _mm256_loadu_pd(address);
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_loadu_pd(address);
#else
	return *address;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Loads a vector of 'complex<double>' values.
// \ingroup simd
//
// \param address The first 'complex<double>' value to be loaded.
// \return The loaded vector of 'complex<double>' values.
//
// This function loads a vector of 'complex<double>' values. In contrast to the according
// \c loada() function, the given address is not required to be properly aligned.
*/
METRIC_NUMERIC_ALWAYS_INLINE const SIMDcdouble loadu(const complex<double> *address) noexcept
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<double>) == 2UL * sizeof(double));

#if METRIC_NUMERIC_AVX512F_MODE
	return _mm512_loadu_pd(address);
#elif METRIC_NUMERIC_MIC_MODE
	__m512d v1 = _mm512_setzero_pd();
	v1 = _mm512_loadunpacklo_pd(v1, reinterpret_cast<const double *>(address));
	v1 = _mm512_loadunpackhi_pd(v1, reinterpret_cast<const double *>(address + 4UL));
	return v1;
#elif METRIC_NUMERIC_AVX_MODE
	return _mm256_loadu_pd(reinterpret_cast<const double *>(address));
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_loadu_pd(reinterpret_cast<const double *>(address));
#else
	return *address;
#endif
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
