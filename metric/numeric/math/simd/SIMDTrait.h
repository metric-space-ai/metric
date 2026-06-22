// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_SIMDTRAIT_H
#define METRIC_NUMERIC_MATH_SIMD_SIMDTRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/simd/BasicTypes.h>
#include <metric/numeric/system/Vectorization.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/StaticAssert.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/HasSize.h>
#include <metric/numeric/util/typetraits/IsIntegral.h>
#include <metric/numeric/util/typetraits/IsNumeric.h>
#include <metric/numeric/util/typetraits/IsSigned.h>
#include <metric/numeric/util/typetraits/RemoveCV.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS SIMDTRAITBASE
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Base template for the SIMDTraitBase class.
// \ingroup simd
*/
template <typename T, typename = void> struct SIMDTraitBase {
	using Type = T;
	static constexpr size_t size = 1UL;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SIMDTraitBase class template for 1-byte integral data types.
// \ingroup simd
*/
template <typename T> struct SIMDTraitBase<T, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T> && Has1Byte_v<T>>> {
	using Type = If_t<IsSigned_v<T>, SIMDint8, SIMDuint8>;
	static constexpr size_t size = Type::size;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SIMDTraitBase class template for 1-byte integral complex data types.
// \ingroup simd
*/
template <typename T> struct SIMDTraitBase<complex<T>, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T> && Has1Byte_v<T>>> {
	using Type = If_t<IsSigned_v<T>, SIMDcint8, SIMDcuint8>;
	static constexpr size_t size = Type::size;

	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<T>) == 2UL * sizeof(T));
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SIMDTraitBase class template for 2-byte integral data types.
// \ingroup simd
*/
template <typename T> struct SIMDTraitBase<T, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T> && Has2Bytes_v<T>>> {
	using Type = If_t<IsSigned_v<T>, SIMDint16, SIMDuint16>;
	static constexpr size_t size = Type::size;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SIMDTraitBase class template for 2-byte integral complex data types.
// \ingroup simd
*/
template <typename T>
struct SIMDTraitBase<complex<T>, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T> && Has2Bytes_v<T>>> {
	using Type = If_t<IsSigned_v<T>, SIMDcint16, SIMDcuint16>;
	static constexpr size_t size = Type::size;

	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<T>) == 2UL * sizeof(T));
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SIMDTraitBase class template for 4-byte integral data types.
// \ingroup simd
*/
template <typename T> struct SIMDTraitBase<T, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T> && Has4Bytes_v<T>>> {
	using Type = If_t<IsSigned_v<T>, SIMDint32, SIMDuint32>;
	static constexpr size_t size = Type::size;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SIMDTraitBase class template for 4-byte integral complex data types.
// \ingroup simd
*/
template <typename T>
struct SIMDTraitBase<complex<T>, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T> && Has4Bytes_v<T>>> {
	using Type = If_t<IsSigned_v<T>, SIMDcint32, SIMDcuint32>;
	static constexpr size_t size = Type::size;

	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<T>) == 2UL * sizeof(T));
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SIMDTraitBase class template for 8-byte integral data types.
// \ingroup simd
*/
template <typename T> struct SIMDTraitBase<T, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T> && Has8Bytes_v<T>>> {
	using Type = If_t<IsSigned_v<T>, SIMDint64, SIMDuint64>;
	static constexpr size_t size = Type::size;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SIMDTraitBase class template for 8-byte integral complex data types.
// \ingroup simd
*/
template <typename T>
struct SIMDTraitBase<complex<T>, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T> && Has8Bytes_v<T>>> {
	using Type = If_t<IsSigned_v<T>, SIMDcint64, SIMDcuint64>;
	static constexpr size_t size = Type::size;

	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<T>) == 2UL * sizeof(T));
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SIMDTraitBase class template for 'float'.
// \ingroup simd
*/
template <> struct SIMDTraitBase<float> {
	using Type = SIMDfloat;
	static constexpr size_t size = Type::size;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SIMDTraitBase class template for 'complex<float>'.
// \ingroup simd
*/
template <> struct SIMDTraitBase<complex<float>> {
	using Type = SIMDcfloat;
	static constexpr size_t size = Type::size;

	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<float>) == 2UL * sizeof(float));
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SIMDTraitBase class template for 'double'.
// \ingroup simd
*/
template <> struct SIMDTraitBase<double> {
	using Type = SIMDdouble;
	static constexpr size_t size = Type::size;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SIMDTraitBase class template for 'complex<double>'.
// \ingroup simd
*/
template <> struct SIMDTraitBase<complex<double>> {
	using Type = SIMDcdouble;
	static constexpr size_t size = Type::size;

	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<double>) == 2UL * sizeof(double));
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  CLASS SIMDTRAIT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief SIMD characteristics of data types.
// \ingroup simd
//
// The SIMDTrait class template provides the SIMD characteristics of a specific data type:
//
//  - The nested data type \a Type corresponds to the according packed, SIMD data type. In case
//    the data type doesn't have a SIMD representation, \a Type corresonds to the given data
//    type itself.
//  - The \a size member constant corresponds to the number of values of the given data type that
//    are packed together in one SIMD vector type. In case the data type cannot be vectorized,
//    \a size is set to 1.
*/
template <typename T> class SIMDTrait : public SIMDTraitBase<RemoveCV_t<T>> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the SIMDTrait class template.
// \ingroup simd
//
// The SIMDTrait_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the SIMDTrait class template. For instance, given the type \a T the following two type
// definitions are identical:

   \code
   using Type1 = typename SIMDTrait<T>::Type;
   using Type2 = SIMDTrait_t<T>;
   \endcode
*/
template <typename T> using SIMDTrait_t = typename SIMDTrait<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
