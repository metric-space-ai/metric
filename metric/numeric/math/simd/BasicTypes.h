// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_BASICTYPES_H
#define METRIC_NUMERIC_MATH_SIMD_BASICTYPES_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/simd/SIMDPack.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/system/Vectorization.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  8-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*\class mtrc::numeric::SIMDi8
// \brief Base class for all 8-bit integral SIMD data types.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> // Type of the SIMD element
struct SIMDi8 : public SIMDPack<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDint8
// \brief SIMD type for 8-bit signed integral data values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDint8 : public SIMDi8<SIMDint8> {
	using This = SIMDint8;
	using BaseType = SIMDi8<This>;
	using ValueType = int8_t;

#if METRIC_NUMERIC_AVX512BW_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint8() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 64UL;
#elif METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint8() noexcept : value(0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint8() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 32UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint8() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 16UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint8() noexcept : value(0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDint8(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDint8(const SIMDi8<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDint8 &operator=(const SIMDi8<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDuint8
// \brief SIMD type for 8-bit unsigned integral data values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDuint8 : public SIMDi8<SIMDuint8> {
	using This = SIMDuint8;
	using BaseType = SIMDi8<This>;
	using ValueType = uint8_t;

#if METRIC_NUMERIC_AVX512BW_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint8() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 64UL;
#elif METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint8() noexcept : value(0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint8() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 32UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint8() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 16UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint8() noexcept : value(0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint8(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDuint8(const SIMDi8<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDuint8 &operator=(const SIMDi8<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  8-BIT INTEGRAL COMPLEX SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*\class mtrc::numeric::SIMDci8
// \brief Base class for all 8-bit integral complex SIMD data types.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> // Type of the SIMD element
struct SIMDci8 : public SIMDPack<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDcint8
// \brief SIMD type for 8-bit signed integral complex values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDcint8 : public SIMDci8<SIMDcint8> {
	using This = SIMDcint8;
	using BaseType = SIMDci8<This>;
	using ValueType = complex<int8_t>;

#if METRIC_NUMERIC_AVX512BW_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint8() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 32UL;
#elif METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint8() noexcept : value(0, 0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint8() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 16UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint8() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint8() noexcept : value(0, 0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint8(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcint8(const SIMDci8<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcint8 &operator=(const SIMDci8<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDcuint8
// \brief SIMD type for 8-bit unsigned integral complex values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDcuint8 : public SIMDci8<SIMDcuint8> {
	using This = SIMDcuint8;
	using BaseType = SIMDci8<This>;
	using ValueType = complex<uint8_t>;

#if METRIC_NUMERIC_AVX512BW_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint8() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 32UL;
#elif METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint8() noexcept : value(0, 0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint8() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 16UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint8() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint8() noexcept : value(0, 0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint8(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint8(const SIMDci8<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint8 &operator=(const SIMDci8<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  16-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*\class mtrc::numeric::SIMDi16
// \brief Base class for all 16-bit integral SIMD data types.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> // Type of the SIMD element
struct SIMDi16 : public SIMDPack<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDint16
// \brief SIMD type for 16-bit signed integral data values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDint16 : public SIMDi16<SIMDint16> {
	using This = SIMDint16;
	using BaseType = SIMDi16<This>;
	using ValueType = int16_t;

#if METRIC_NUMERIC_AVX512BW_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint16() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 32UL;
#elif METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint16() noexcept : value(0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint16() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 16UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint16() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint16() noexcept : value(0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDint16(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDint16(const SIMDi16<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDint16 &operator=(const SIMDi16<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDuint16
// \brief SIMD type for 16-bit unsigned integral data values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDuint16 : public SIMDi16<SIMDuint16> {
	using This = SIMDuint16;
	using BaseType = SIMDi16<This>;
	using ValueType = uint16_t;

#if METRIC_NUMERIC_AVX512BW_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint16() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 32UL;
#elif METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint16() noexcept : value(0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint16() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 16UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint16() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint16() noexcept : value(0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint16(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDuint16(const SIMDi16<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDuint16 &operator=(const SIMDi16<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  16-BIT INTEGRAL COMPLEX SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*\class mtrc::numeric::SIMDci16
// \brief Base class for all 16-bit integral complex SIMD data types.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> // Type of the SIMD element
struct SIMDci16 : public SIMDPack<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDcint16
// \brief SIMD type for 16-bit signed integral complex values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDcint16 : public SIMDci16<SIMDcint16> {
	using This = SIMDcint16;
	using BaseType = SIMDci16<This>;
	using ValueType = complex<int16_t>;

#if METRIC_NUMERIC_AVX512BW_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint16() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 16UL;
#elif METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint16() noexcept : value(0, 0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint16() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint16() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 4UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint16() noexcept : value(0, 0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint16(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcint16(const SIMDci16<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcint16 &operator=(const SIMDci16<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDcuint16
// \brief SIMD type for 16-bit unsigned integral complex values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDcuint16 : public SIMDci16<SIMDcuint16> {
	using This = SIMDcuint16;
	using BaseType = SIMDci16<This>;
	using ValueType = complex<uint16_t>;

#if METRIC_NUMERIC_AVX512BW_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint16() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 16UL;
#elif METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint16() noexcept : value(0, 0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint16() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint16() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 4UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint16() noexcept : value(0, 0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint16(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint16(const SIMDci16<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint16 &operator=(const SIMDci16<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  32-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*\class mtrc::numeric::SIMDi32
// \brief Base class for all 32-bit integral SIMD data types.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> // Type of the SIMD element
struct SIMDi32 : public SIMDPack<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDint32
// \brief SIMD type for 32-bit signed integral data values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDint32 : public SIMDi32<SIMDint32> {
	using This = SIMDint32;
	using BaseType = SIMDi32<This>;
	using ValueType = int32_t;

#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint32() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 16UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint32() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint32() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 4UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint32() noexcept : value(0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDint32(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDint32(const SIMDi32<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDint32 &operator=(const SIMDi32<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDuint32
// \brief SIMD type for 32-bit unsigned integral data values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDuint32 : public SIMDi32<SIMDuint32> {
	using This = SIMDuint32;
	using BaseType = SIMDi32<This>;
	using ValueType = uint32_t;

#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint32() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 16UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint32() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint32() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 4UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint32() noexcept : value(0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint32(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDuint32(const SIMDi32<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDuint32 &operator=(const SIMDi32<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  32-BIT INTEGRAL COMPLEX SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*\class mtrc::numeric::SIMDci32
// \brief Base class for all 32-bit integral complex SIMD data types.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> // Type of the SIMD element
struct SIMDci32 : public SIMDPack<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDcint32
// \brief SIMD type for 32-bit signed integral complex values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDcint32 : public SIMDci32<SIMDcint32> {
	using This = SIMDcint32;
	using BaseType = SIMDci32<This>;
	using ValueType = complex<int32_t>;

#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint32() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint32() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 4UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint32() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 2UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint32() noexcept : value(0, 0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint32(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcint32(const SIMDci32<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcint32 &operator=(const SIMDci32<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDcuint32
// \brief SIMD type for 32-bit unsigned integral complex values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDcuint32 : public SIMDci32<SIMDcuint32> {
	using This = SIMDcuint32;
	using BaseType = SIMDci32<This>;
	using ValueType = complex<uint32_t>;

#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint32() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint32() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 4UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint32() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 2UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint32() noexcept : value(0, 0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint32(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint32(const SIMDci32<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint32 &operator=(const SIMDci32<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  64-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*\class mtrc::numeric::SIMDi64
// \brief Base class for all 64-bit integral SIMD data types.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> // Type of the SIMD element
struct SIMDi64 : public SIMDPack<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDint64
// \brief SIMD type for 64-bit integral data values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDint64 : public SIMDi64<SIMDint64> {
	using This = SIMDint64;
	using BaseType = SIMDi64<This>;
	using ValueType = int64_t;

#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint64() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint64() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 4UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint64() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 2UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDint64() noexcept : value(0L) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDint64(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDint64(const SIMDi64<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDint64 &operator=(const SIMDi64<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDuint64
// \brief SIMD type for 64-bit unsigned integral data values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDuint64 : public SIMDi64<SIMDuint64> {
	using This = SIMDuint64;
	using BaseType = SIMDi64<This>;
	using ValueType = uint64_t;

#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint64() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint64() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 4UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint64() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 2UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint64() noexcept : value(0L) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDuint64(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDuint64(const SIMDi64<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDuint64 &operator=(const SIMDi64<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  64-BIT INTEGRAL COMPLEX SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*\class mtrc::numeric::SIMDci64
// \brief Base class for all 64-bit integral complex SIMD data types.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> // Type of the SIMD element
struct SIMDci64 : public SIMDPack<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDcint64
// \brief SIMD type for 64-bit signed integral complex values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDcint64 : public SIMDci64<SIMDcint64> {
	using This = SIMDcint64;
	using BaseType = SIMDci64<This>;
	using ValueType = complex<int64_t>;

#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint64() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 4UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint64() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 2UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint64() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 1UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint64() noexcept : value(0L, 0L) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDcint64(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcint64(const SIMDci64<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcint64 &operator=(const SIMDci64<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDcuint64
// \brief SIMD type for 64-bit unsigned integral complex values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDcuint64 : public SIMDci64<SIMDcuint64> {
	using This = SIMDcuint64;
	using BaseType = SIMDci64<This>;
	using ValueType = complex<uint64_t>;

#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = __m512i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint64() noexcept : value(_mm512_setzero_si512()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 4UL;
#elif METRIC_NUMERIC_AVX2_MODE
	using IntrinsicType = __m256i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint64() noexcept : value(_mm256_setzero_si256()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 2UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128i;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint64() noexcept : value(_mm_setzero_si128()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 1UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint64() noexcept : value(0L, 0L) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint64(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint64(const SIMDci64<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcuint64 &operator=(const SIMDci64<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SINGLE PRECISION FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*\class mtrc::numeric::SIMDf32
// \brief Base class for all single precision floating point SIMD data types.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> // Type of the SIMD element
struct SIMDf32 : public SIMDPack<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDfloat
// \brief SIMD type for 32-bit single precision floating point data values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDfloat : public SIMDf32<SIMDfloat> {
	using This = SIMDfloat;
	using BaseType = SIMDf32<This>;
	using ValueType = float;

#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = __m512;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDfloat() noexcept : value(_mm512_setzero_ps()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 16UL;
#elif METRIC_NUMERIC_AVX_MODE
	using IntrinsicType = __m256;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDfloat() noexcept : value(_mm256_setzero_ps()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#elif METRIC_NUMERIC_SSE_MODE
	using IntrinsicType = __m128;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDfloat() noexcept : value(_mm_setzero_ps()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 4UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDfloat() noexcept : value(0.0F) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDfloat(IntrinsicType v) noexcept : value(v) {}

	template <typename T>
	METRIC_NUMERIC_ALWAYS_INLINE SIMDfloat(const SIMDf32<T> &v) noexcept : value((*v).eval().value)
	{
	}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDfloat &operator=(const SIMDf32<T> &v) noexcept
	{
		value = (*v).eval().value;
		return *this;
	}

	METRIC_NUMERIC_ALWAYS_INLINE const This &eval() const { return *this; }

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SINGLE PRECISION FLOATING POINT COMPLEX SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*\class mtrc::numeric::SIMDcf32
// \brief Base class for all single precision floating point complex SIMD data types.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> // Type of the SIMD element
struct SIMDcf32 : public SIMDPack<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDcfloat
// \brief SIMD type for 32-bit single precision complex values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDcfloat : public SIMDcf32<SIMDcfloat> {
	using This = SIMDcfloat;
	using BaseType = SIMDcf32<This>;
	using ValueType = complex<float>;

#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = __m512;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcfloat() noexcept : value(_mm512_setzero_ps()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#elif METRIC_NUMERIC_AVX_MODE
	using IntrinsicType = __m256;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcfloat() noexcept : value(_mm256_setzero_ps()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 4UL;
#elif METRIC_NUMERIC_SSE_MODE
	using IntrinsicType = __m128;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcfloat() noexcept : value(_mm_setzero_ps()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 2UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcfloat() noexcept : value(0.0F, 0.0F) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDcfloat(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcfloat(const SIMDcf32<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcfloat &operator=(const SIMDcf32<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DOUBLE PRECISION FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*\class mtrc::numeric::SIMDf64
// \brief Base class for all double precision floating point SIMD data types.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> // Type of the SIMD element
struct SIMDf64 : public SIMDPack<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDdouble
// \brief SIMD type for 64-bit double precision floating point data values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDdouble : public SIMDf64<SIMDdouble> {
	using This = SIMDdouble;
	using BaseType = SIMDf64<This>;
	using ValueType = double;

#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = __m512d;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDdouble() noexcept : value(_mm512_setzero_pd()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 8UL;
#elif METRIC_NUMERIC_AVX_MODE
	using IntrinsicType = __m256d;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDdouble() noexcept : value(_mm256_setzero_pd()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 4UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128d;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDdouble() noexcept : value(_mm_setzero_pd()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 2UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDdouble() noexcept : value(0.0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDdouble(IntrinsicType v) noexcept : value(v) {}

	template <typename T>
	METRIC_NUMERIC_ALWAYS_INLINE SIMDdouble(const SIMDf64<T> &v) noexcept : value((*v).eval().value)
	{
	}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDdouble &operator=(const SIMDf64<T> &v) noexcept
	{
		value = (*v).eval().value;
		return *this;
	}

	METRIC_NUMERIC_ALWAYS_INLINE const This &eval() const { return *this; }

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DOUBLE PRECISION FLOATING POINT COMPLEX SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*\class mtrc::numeric::SIMDcf64
// \brief Base class for all double precision floating point complex SIMD data types.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> // Type of the SIMD element
struct SIMDcf64 : public SIMDPack<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::SIMDcdouble
// \brief SIMD type for 64-bit double precision complex values.
// \ingroup simd
*/
/*! \cond METRIC_NUMERIC_INTERNAL */
struct SIMDcdouble : public SIMDcf64<SIMDcdouble> {
	using This = SIMDcdouble;
	using BaseType = SIMDcf64<This>;
	using ValueType = complex<double>;

#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
	using IntrinsicType = __m512d;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcdouble() noexcept : value(_mm512_setzero_pd()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 4UL;
#elif METRIC_NUMERIC_AVX_MODE
	using IntrinsicType = __m256d;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcdouble() noexcept : value(_mm256_setzero_pd()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 2UL;
#elif METRIC_NUMERIC_SSE2_MODE
	using IntrinsicType = __m128d;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcdouble() noexcept : value(_mm_setzero_pd()) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t i) const noexcept
	{
		return reinterpret_cast<const ValueType *>(&value)[i];
	}
	static constexpr size_t size = 1UL;
#else
	using IntrinsicType = ValueType;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDcdouble() noexcept : value(0.0, 0.0) {}
	METRIC_NUMERIC_ALWAYS_INLINE ValueType operator[](size_t /*i*/) const noexcept { return value; }
	static constexpr size_t size = 1UL;
#endif

	METRIC_NUMERIC_ALWAYS_INLINE SIMDcdouble(IntrinsicType v) noexcept : value(v) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcdouble(const SIMDcf64<T> &v) noexcept : value((*v).value) {}

	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE SIMDcdouble &operator=(const SIMDcf64<T> &v) noexcept
	{
		value = (*v).value;
		return *this;
	}

	IntrinsicType value;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SIMD OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Addition assignment operator for the addition of two SIMD packs.
// \ingroup simd
//
// \param lhs The left-hand side SIMD operand for the addition.
// \param rhs The right-hand side SIMD operand for the addition.
// \return Reference to the left-hand side SIMD operand.
*/
template <typename T1 // Type of the left-hand side SIMD operand
		  ,
		  typename T2> // Type of the right-hand side SIMD operand
METRIC_NUMERIC_ALWAYS_INLINE T1 &operator+=(SIMDPack<T1> &lhs, const SIMDPack<T2> &rhs)
{
	(*lhs) = (*lhs) + (*rhs);
	return *lhs;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Subtraction assignment operator for the subtraction of two SIMD packs.
// \ingroup simd
//
// \param lhs The left-hand side SIMD operand for the subtraction.
// \param rhs The right-hand side SIMD operand for the subtraction.
// \return Reference to the left-hand side SIMD operand.
*/
template <typename T1 // Type of the left-hand side SIMD operand
		  ,
		  typename T2> // Type of the right-hand side SIMD operand
METRIC_NUMERIC_ALWAYS_INLINE T1 &operator-=(SIMDPack<T1> &lhs, const SIMDPack<T2> &rhs)
{
	(*lhs) = (*lhs) - (*rhs);
	return *lhs;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Multiplication assignment operator for the multiplication of two SIMD packs.
// \ingroup simd
//
// \param lhs The left-hand side SIMD operand for the multiplication.
// \param rhs The right-hand side SIMD operand for the multiplication.
// \return Reference to the left-hand side SIMD operand.
*/
template <typename T1 // Type of the left-hand side SIMD operand
		  ,
		  typename T2> // Type of the right-hand side SIMD operand
METRIC_NUMERIC_ALWAYS_INLINE T1 &operator*=(SIMDPack<T1> &lhs, const SIMDPack<T2> &rhs)
{
	(*lhs) = (*lhs) * (*rhs);
	return *lhs;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Division assignment operator for the division of two SIMD packs.
// \ingroup simd
//
// \param lhs The left-hand side SIMD operand for the division.
// \param rhs The right-hand side SIMD operand for the division.
// \return Reference to the left-hand side SIMD operand.
*/
template <typename T1 // Type of the left-hand side SIMD operand
		  ,
		  typename T2> // Type of the right-hand side SIMD operand
METRIC_NUMERIC_ALWAYS_INLINE T1 &operator/=(SIMDPack<T1> &lhs, const SIMDPack<T2> &rhs)
{
	(*lhs) = (*lhs) / (*rhs);
	return *lhs;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
