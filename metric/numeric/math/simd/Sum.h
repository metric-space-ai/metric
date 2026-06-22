// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_SUM_H
#define METRIC_NUMERIC_MATH_SIMD_SUM_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
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
/*!\brief Returns the sum of all elements in the 8-bit integral SIMD vector.
// \ingroup simd
//
// \param a The vector to be summed up.
// \return The sum of all vector elements.
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE ValueType_t<T> sum(const SIMDi8<T> &a) noexcept
{
#if METRIC_NUMERIC_AVX512BW_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3] + (*a)[4] + (*a)[5] + (*a)[6] + (*a)[7] + (*a)[8] + (*a)[9] +
		   (*a)[10] + (*a)[11] + (*a)[12] + (*a)[13] + (*a)[14] + (*a)[15] + (*a)[16] + (*a)[17] + (*a)[18] + (*a)[19] +
		   (*a)[20] + (*a)[21] + (*a)[22] + (*a)[23] + (*a)[24] + (*a)[25] + (*a)[26] + (*a)[27] + (*a)[28] + (*a)[29] +
		   (*a)[30] + (*a)[31] + (*a)[32] + (*a)[33] + (*a)[34] + (*a)[35] + (*a)[36] + (*a)[37] + (*a)[38] + (*a)[39] +
		   (*a)[40] + (*a)[41] + (*a)[42] + (*a)[43] + (*a)[44] + (*a)[45] + (*a)[46] + (*a)[47] + (*a)[48] + (*a)[49] +
		   (*a)[50] + (*a)[51] + (*a)[52] + (*a)[53] + (*a)[54] + (*a)[55] + (*a)[56] + (*a)[57] + (*a)[58] + (*a)[59] +
		   (*a)[60] + (*a)[61] + (*a)[62] + (*a)[63];
#elif METRIC_NUMERIC_AVX2_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3] + (*a)[4] + (*a)[5] + (*a)[6] + (*a)[7] + (*a)[8] + (*a)[9] +
		   (*a)[10] + (*a)[11] + (*a)[12] + (*a)[13] + (*a)[14] + (*a)[15] + (*a)[16] + (*a)[17] + (*a)[18] + (*a)[19] +
		   (*a)[20] + (*a)[21] + (*a)[22] + (*a)[23] + (*a)[24] + (*a)[25] + (*a)[26] + (*a)[27] + (*a)[28] + (*a)[29] +
		   (*a)[30] + (*a)[31];
#elif METRIC_NUMERIC_SSE2_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3] + (*a)[4] + (*a)[5] + (*a)[6] + (*a)[7] + (*a)[8] + (*a)[9] +
		   (*a)[10] + (*a)[11] + (*a)[12] + (*a)[13] + (*a)[14] + (*a)[15];
#else
	return (*a).value;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the sum of all elements in the 8-bit integral complex SIMD vector.
// \ingroup simd
//
// \param a The vector to be summed up.
// \return The sum of all vector elements.
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE const ValueType_t<T> sum(const SIMDci8<T> &a) noexcept
{
#if METRIC_NUMERIC_AVX512BW_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3] + (*a)[4] + (*a)[5] + (*a)[6] + (*a)[7] + (*a)[8] + (*a)[9] +
		   (*a)[10] + (*a)[11] + (*a)[12] + (*a)[13] + (*a)[14] + (*a)[15] + (*a)[16] + (*a)[17] + (*a)[18] + (*a)[19] +
		   (*a)[20] + (*a)[21] + (*a)[22] + (*a)[23] + (*a)[24] + (*a)[25] + (*a)[26] + (*a)[27] + (*a)[28] + (*a)[29] +
		   (*a)[30] + (*a)[31];
#elif METRIC_NUMERIC_AVX2_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3] + (*a)[4] + (*a)[5] + (*a)[6] + (*a)[7] + (*a)[8] + (*a)[9] +
		   (*a)[10] + (*a)[11] + (*a)[12] + (*a)[13] + (*a)[14] + (*a)[15];
#elif METRIC_NUMERIC_SSE2_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3] + (*a)[4] + (*a)[5] + (*a)[6] + (*a)[7];
#else
	return (*a).value;
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  16-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the sum of all elements in the 16-bit integral SIMD vector.
// \ingroup simd
//
// \param a The vector to be summed up.
// \return The sum of all vector elements.
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE ValueType_t<T> sum(const SIMDi16<T> &a) noexcept
{
#if METRIC_NUMERIC_AVX512BW_MODE
	const __m256i low(_mm512_castsi512_si256((*a).value));
	const __m256i high(_mm512_extracti64x4_epi64((*a).value, 1));
	const __m256i b(_mm256_hadd_epi16(low, high));
	const __m256i c(_mm256_hadd_epi16(b, b));
	const __m256i d(_mm256_hadd_epi16(c, c));
	const __m256i e(_mm256_hadd_epi16(d, d));
	const __m128i f(_mm_add_epi16(_mm256_extracti128_si256(e, 1), _mm256_castsi256_si128(e)));
	return _mm_extract_epi16(f, 0);
#elif METRIC_NUMERIC_AVX2_MODE
	const __m256i b(_mm256_hadd_epi16((*a).value, (*a).value));
	const __m256i c(_mm256_hadd_epi16(b, b));
	const __m256i d(_mm256_hadd_epi16(c, c));
	const __m128i e(_mm_add_epi16(_mm256_extracti128_si256(d, 1), _mm256_castsi256_si128(d)));
	return _mm_extract_epi16(e, 0);
#elif METRIC_NUMERIC_SSSE3_MODE
	const __m128i b(_mm_hadd_epi16((*a).value, (*a).value));
	const __m128i c(_mm_hadd_epi16(b, b));
	const __m128i d(_mm_hadd_epi16(c, c));
	return _mm_extract_epi16(d, 0);
#elif METRIC_NUMERIC_SSE2_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3] + (*a)[4] + (*a)[5] + (*a)[6] + (*a)[7];
#else
	return (*a).value;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the sum of all elements in the 16-bit integral complex SIMD vector.
// \ingroup simd
//
// \param a The vector to be summed up.
// \return The sum of all vector elements.
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE const ValueType_t<T> sum(const SIMDci16<T> &a) noexcept
{
#if METRIC_NUMERIC_AVX512BW_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3] + (*a)[4] + (*a)[5] + (*a)[6] + (*a)[7] + (*a)[8] + (*a)[9] +
		   (*a)[10] + (*a)[11] + (*a)[12] + (*a)[13] + (*a)[14] + (*a)[15];
#elif METRIC_NUMERIC_AVX2_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3] + (*a)[4] + (*a)[5] + (*a)[6] + (*a)[7];
#elif METRIC_NUMERIC_SSE2_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3];
#else
	return (*a).value;
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  32-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the sum of all elements in the 32-bit integral SIMD vector.
// \ingroup simd
//
// \param a The vector to be summed up.
// \return The sum of all vector elements.
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE ValueType_t<T> sum(const SIMDi32<T> &a) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE
	const __m256i low(_mm512_castsi512_si256((*a).value));
	const __m256i high(_mm512_extracti64x4_epi64((*a).value, 1));
	const __m256i b(_mm256_hadd_epi32(low, high));
	const __m256i c(_mm256_hadd_epi32(b, b));
	const __m256i d(_mm256_hadd_epi32(c, c));
	const __m128i e(_mm_add_epi32(_mm256_extracti128_si256(d, 1), _mm256_castsi256_si128(d)));
	return _mm_extract_epi32(e, 0);
#elif METRIC_NUMERIC_MIC_MODE
	return _mm512_reduce_add_epi32((*a).value);
#elif METRIC_NUMERIC_AVX2_MODE
	const __m256i b(_mm256_hadd_epi32((*a).value, (*a).value));
	const __m256i c(_mm256_hadd_epi32(b, b));
	const __m128i d(_mm_add_epi32(_mm256_extracti128_si256(c, 1), _mm256_castsi256_si128(c)));
	return _mm_extract_epi32(d, 0);
#elif METRIC_NUMERIC_SSSE3_MODE
	const __m128i b(_mm_hadd_epi32((*a).value, (*a).value));
	return _mm_cvtsi128_si32(_mm_hadd_epi32(b, b));
#elif METRIC_NUMERIC_SSE2_MODE
	const __m128i b(_mm_add_epi32((*a).value, _mm_shuffle_epi32((*a).value, 0x4E)));
	return _mm_cvtsi128_si32(_mm_add_epi32(b, _mm_shuffle_epi32(b, 0xB1)));
#else
	return (*a).value;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the sum of all elements in the 32-bit integral complex SIMD vector.
// \ingroup simd
//
// \param a The vector to be summed up.
// \return The sum of all vector elements.
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE const ValueType_t<T> sum(const SIMDci32<T> &a) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3] + (*a)[4] + (*a)[5] + (*a)[6] + (*a)[7];
#elif METRIC_NUMERIC_AVX2_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3];
#elif METRIC_NUMERIC_SSE2_MODE
	return (*a)[0] + (*a)[1];
#else
	return (*a).value;
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  64-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the sum of all elements in the 64-bit integral SIMD vector.
// \ingroup simd
//
// \param a The vector to be summed up.
// \return The sum of all vector elements.
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE ValueType_t<T> sum(const SIMDi64<T> &a) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3] + (*a)[4] + (*a)[5] + (*a)[6] + (*a)[7];
#elif METRIC_NUMERIC_AVX2_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3];
#elif METRIC_NUMERIC_SSE2_MODE
	return (*a)[0] + (*a)[1];
#else
	return (*a).value;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the sum of all elements in the 64-bit integral complex SIMD vector.
// \ingroup simd
//
// \param a The vector to be summed up.
// \return The sum of all vector elements.
*/
template <typename T> // Type of the SIMD element
METRIC_NUMERIC_ALWAYS_INLINE const ValueType_t<T> sum(const SIMDci64<T> &a) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return (*a)[0] + (*a)[1] + (*a)[2] + (*a)[3];
#elif METRIC_NUMERIC_AVX2_MODE
	return (*a)[0] + (*a)[1];
#elif METRIC_NUMERIC_SSE2_MODE
	return (*a)[0];
#else
	return (*a).value;
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  32-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the sum of all elements in the single precision floating point SIMD vector.
// \ingroup simd
//
// \param a The vector to be summed up.
// \return The sum of all vector elements.
*/
METRIC_NUMERIC_ALWAYS_INLINE float sum(const SIMDfloat &a) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return _mm512_reduce_add_ps(a.value);
#elif METRIC_NUMERIC_AVX_MODE
	const __m128 b(_mm_add_ps(_mm256_extractf128_ps(a.value, 1), _mm256_castps256_ps128(a.value)));
	const __m128 c(_mm_add_ps(b, _mm_movehl_ps(b, b)));
	return _mm_cvtss_f32(_mm_add_ss(c, _mm_shuffle_ps(c, c, 1)));
#elif METRIC_NUMERIC_SSE3_MODE
	const __m128 b(_mm_add_ps(a.value, _mm_movehl_ps(a.value, a.value)));
	return _mm_cvtss_f32(_mm_add_ss(b, _mm_shuffle_ps(b, b, 1)));
#elif METRIC_NUMERIC_SSE_MODE
	const __m128 b(_mm_add_ps(a.value, _mm_movehl_ps(a.value, a.value)));
	return _mm_cvtss_f32(_mm_add_ss(b, _mm_shuffle_ps(b, b, 1)));
#else
	return a.value;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the sum of all elements in the single precision complex SIMD vector.
// \ingroup simd
//
// \param a The vector to be summed up.
// \return The sum of all vector elements.
*/
METRIC_NUMERIC_ALWAYS_INLINE const complex<float> sum(const SIMDcfloat &a) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return a[0] + a[1] + a[2] + a[3] + a[4] + a[5] + a[6] + a[7];
#elif METRIC_NUMERIC_AVX_MODE
	return a[0] + a[1] + a[2] + a[3];
#elif METRIC_NUMERIC_SSE_MODE
	return a[0] + a[1];
#else
	return a.value;
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  64-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the sum of all elements in the double precision floating point SIMD vector.
// \ingroup simd
//
// \param a The vector to be summed up.
// \return The sum of all vector elements.
*/
METRIC_NUMERIC_ALWAYS_INLINE double sum(const SIMDdouble &a) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return _mm512_reduce_add_pd(a.value);
#elif METRIC_NUMERIC_AVX_MODE
	const __m128d b(_mm_add_pd(_mm256_castpd256_pd128(a.value), _mm256_extractf128_pd(a.value, 1)));
	return _mm_cvtsd_f64(_mm_add_sd(b, _mm_unpackhi_pd(b, b)));
#elif METRIC_NUMERIC_SSE2_MODE
	return _mm_cvtsd_f64(_mm_add_sd(a.value, _mm_unpackhi_pd(a.value, a.value)));
#else
	return a.value;
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the sum of all elements in the double precision complex SIMD vector.
// \ingroup simd
//
// \param a The vector to be summed up.
// \return The sum of all vector elements.
*/
METRIC_NUMERIC_ALWAYS_INLINE const complex<double> sum(const SIMDcdouble &a) noexcept
{
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	return a[0] + a[1] + a[2] + a[3];
#elif METRIC_NUMERIC_AVX_MODE
	return a[0] + a[1];
#elif METRIC_NUMERIC_SSE2_MODE
	return a[0];
#else
	return a.value;
#endif
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
