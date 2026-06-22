// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDMAX_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDMAX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Vectorization.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/HasSize.h>
#include <metric/numeric/util/typetraits/IsIntegral.h>
#include <metric/numeric/util/typetraits/IsNumeric.h>
#include <metric/numeric/util/typetraits/IsSigned.h>
#include <metric/numeric/util/typetraits/IsUnsigned.h>
#include <metric/numeric/util/typetraits/RemoveCVRef.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the HasSIMDMax type trait.
// \ingroup math_type_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename = void> // Restricting condition
struct HasSIMDMaxHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T>
struct HasSIMDMaxHelper<T, T, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T> && Has1Byte_v<T>>>
	: public BoolConstant<(bool(METRIC_NUMERIC_SSE2_MODE) && IsUnsigned_v<T>) ||
						  (bool(METRIC_NUMERIC_SSE4_MODE) && IsSigned_v<T>) || bool(METRIC_NUMERIC_AVX2_MODE) ||
						  bool(METRIC_NUMERIC_AVX512BW_MODE)> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T>
struct HasSIMDMaxHelper<T, T, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T> && Has2Bytes_v<T>>>
	: public BoolConstant<(bool(METRIC_NUMERIC_SSE2_MODE) && IsSigned_v<T>) ||
						  (bool(METRIC_NUMERIC_SSE4_MODE) && IsUnsigned_v<T>) || bool(METRIC_NUMERIC_AVX2_MODE) ||
						  bool(METRIC_NUMERIC_AVX512BW_MODE)> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T>
struct HasSIMDMaxHelper<T, T, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T> && Has4Bytes_v<T>>>
	: public BoolConstant<bool(METRIC_NUMERIC_SSE4_MODE) || bool(METRIC_NUMERIC_AVX2_MODE) ||
						  bool(METRIC_NUMERIC_MIC_MODE) || bool(METRIC_NUMERIC_AVX512F_MODE)> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <>
struct HasSIMDMaxHelper<float, float>
	: public BoolConstant<bool(METRIC_NUMERIC_SSE_MODE) || bool(METRIC_NUMERIC_AVX_MODE) ||
						  bool(METRIC_NUMERIC_MIC_MODE) || bool(METRIC_NUMERIC_AVX512F_MODE)> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <>
struct HasSIMDMaxHelper<double, double>
	: public BoolConstant<bool(METRIC_NUMERIC_SSE2_MODE) || bool(METRIC_NUMERIC_AVX_MODE) ||
						  bool(METRIC_NUMERIC_MIC_MODE) || bool(METRIC_NUMERIC_AVX512F_MODE)> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of a SIMD max operation for the given data types.
// \ingroup math_type_traits
//
// Depending on the available instruction set (SSE, SSE2, SSE3, SSE4, AVX, AVX2, MIC, ...), and
// the used compiler, this type trait provides the information whether a SIMD max operation
// exists for the two given data types \a T1 and \a T2 (ignoring the cv-qualifiers). In case the
// SIMD operation is available, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType. The
// following example assumes that AVX is available:

   \code
   mtrc::numeric::HasSIMDMax< int, int >::value     // Evaluates to 1
   mtrc::numeric::HasSIMDMax< float, float >::Type  // Results in TrueType
   mtrc::numeric::HasSIMDMax< double, double >      // Is derived from TrueType
   mtrc::numeric::HasSIMDMax< bool, bool >::value   // Evaluates to 0
   mtrc::numeric::HasSIMDMax< float, int >::Type    // Results in FalseType
   mtrc::numeric::HasSIMDMax< float, double >       // Is derived from FalseType
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename = void> // Restricting condition
struct HasSIMDMax : public BoolConstant<HasSIMDMaxHelper<RemoveCVRef_t<T1>, RemoveCVRef_t<T2>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSIMDMax type trait.
// \ingroup math_type_traits
//
// The HasSIMDMax_v variable template provides a convenient shortcut to access the nested
// \a value of the HasSIMDMax class template. For instance, given the types \a T1 and \a T2
// the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSIMDMax<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HasSIMDMax_v<T1,T2>;
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
constexpr bool HasSIMDMax_v = HasSIMDMax<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
