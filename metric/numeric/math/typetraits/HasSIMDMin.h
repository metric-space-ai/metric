// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDMIN_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDMIN_H
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
/*!\brief Auxiliary helper struct for the HasSIMDMin type trait.
// \ingroup math_type_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename = void> // Restricting condition
struct HasSIMDMinHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T>
struct HasSIMDMinHelper<T, T, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T> && Has1Byte_v<T>>>
	: public BoolConstant<(bool(METRIC_NUMERIC_SSE2_MODE) && IsUnsigned_v<T>) ||
						  (bool(METRIC_NUMERIC_SSE4_MODE) && IsSigned_v<T>) || bool(METRIC_NUMERIC_AVX2_MODE) ||
						  bool(METRIC_NUMERIC_AVX512BW_MODE)> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T>
struct HasSIMDMinHelper<T, T, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T> && Has2Bytes_v<T>>>
	: public BoolConstant<(bool(METRIC_NUMERIC_SSE2_MODE) && IsSigned_v<T>) ||
						  (bool(METRIC_NUMERIC_SSE4_MODE) && IsUnsigned_v<T>) || bool(METRIC_NUMERIC_AVX2_MODE) ||
						  bool(METRIC_NUMERIC_AVX512BW_MODE)> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T>
struct HasSIMDMinHelper<T, T, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T> && Has4Bytes_v<T>>>
	: public BoolConstant<bool(METRIC_NUMERIC_SSE4_MODE) || bool(METRIC_NUMERIC_AVX2_MODE) ||
						  bool(METRIC_NUMERIC_MIC_MODE) || bool(METRIC_NUMERIC_AVX512F_MODE)> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <>
struct HasSIMDMinHelper<float, float>
	: public BoolConstant<bool(METRIC_NUMERIC_SSE_MODE) || bool(METRIC_NUMERIC_AVX_MODE) ||
						  bool(METRIC_NUMERIC_MIC_MODE) || bool(METRIC_NUMERIC_AVX512F_MODE)> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <>
struct HasSIMDMinHelper<double, double>
	: public BoolConstant<bool(METRIC_NUMERIC_SSE2_MODE) || bool(METRIC_NUMERIC_AVX_MODE) ||
						  bool(METRIC_NUMERIC_MIC_MODE) || bool(METRIC_NUMERIC_AVX512F_MODE)> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of a SIMD min operation for the given data types.
// \ingroup math_type_traits
//
// Depending on the available instruction set (SSE, SSE2, SSE3, SSE4, AVX, AVX2, MIC, ...), and
// the used compiler, this type trait provides the information whether a SIMD min operation
// exists for the two given data types \a T1 and \a T2 (ignoring the cv-qualifiers). In case the
// SIMD operation is available, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType. The
// following example assumes that AVX is available:

   \code
   mtrc::numeric::HasSIMDMin< int, int >::value     // Evaluates to 1
   mtrc::numeric::HasSIMDMin< float, float >::Type  // Results in TrueType
   mtrc::numeric::HasSIMDMin< double, double >      // Is derived from TrueType
   mtrc::numeric::HasSIMDMin< bool, bool >::value   // Evaluates to 0
   mtrc::numeric::HasSIMDMin< float, int >::Type    // Results in FalseType
   mtrc::numeric::HasSIMDMin< float, double >       // Is derived from FalseType
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename = void> // Restricting condition
struct HasSIMDMin : public BoolConstant<HasSIMDMinHelper<RemoveCVRef_t<T1>, RemoveCVRef_t<T2>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSIMDMin type trait.
// \ingroup math_type_traits
//
// The HasSIMDMin_v variable template provides a convenient shortcut to access the nested
// \a value of the HasSIMDMin class template. For instance, given the types \a T1 and \a T2
// the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSIMDMin<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HasSIMDMin_v<T1,T2>;
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
constexpr bool HasSIMDMin_v = HasSIMDMin<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
