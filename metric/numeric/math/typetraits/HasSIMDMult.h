// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDMULT_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDMULT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Vectorization.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsIntegral.h>
#include <metric/numeric/util/typetraits/IsNumeric.h>
#include <metric/numeric/util/typetraits/RemoveCVRef.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the HasSIMDMult type trait.
// \ingroup math_type_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename = void> // Restricting condition
struct HasSIMDMultHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
struct HasSIMDMultHelper<
	T1, T2,
	EnableIf_t<IsNumeric_v<T1> && IsIntegral_v<T1> && IsNumeric_v<T2> && IsIntegral_v<T2> && sizeof(T1) == sizeof(T2)>>
	: public BoolConstant<(bool(METRIC_NUMERIC_SSE2_MODE) && sizeof(T1) == 2UL) ||
						  (bool(METRIC_NUMERIC_SSE4_MODE) && sizeof(T1) >= 2UL && sizeof(T1) <= 4UL) ||
						  (bool(METRIC_NUMERIC_AVX2_MODE) && sizeof(T1) >= 2UL && sizeof(T1) <= 4UL) ||
						  (bool(METRIC_NUMERIC_MIC_MODE) && sizeof(T1) == 4UL) ||
						  (bool(METRIC_NUMERIC_AVX512BW_MODE) && sizeof(T1) == 2UL) ||
						  (bool(METRIC_NUMERIC_AVX512F_MODE) && sizeof(T1) == 4UL) ||
						  (bool(METRIC_NUMERIC_AVX512DQ_MODE) && sizeof(T1) == 8UL)> {};

template <typename T>
struct HasSIMDMultHelper<complex<T>, complex<T>, EnableIf_t<IsNumeric_v<T> && IsIntegral_v<T>>>
	: public BoolConstant<(bool(METRIC_NUMERIC_SSE2_MODE) && sizeof(T) == 2UL) ||
						  (bool(METRIC_NUMERIC_SSE4_MODE) && sizeof(T) >= 2UL && sizeof(T) <= 4UL) ||
						  (bool(METRIC_NUMERIC_AVX2_MODE) && sizeof(T) >= 2UL && sizeof(T) <= 4UL) ||
						  (bool(METRIC_NUMERIC_AVX512BW_MODE) && sizeof(T) == 2UL) ||
						  (bool(METRIC_NUMERIC_AVX512F_MODE) && sizeof(T) == 4UL) ||
						  (bool(METRIC_NUMERIC_AVX512DQ_MODE) && sizeof(T) == 8UL)> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <>
struct HasSIMDMultHelper<float, float>
	: public BoolConstant<bool(METRIC_NUMERIC_SSE_MODE) || bool(METRIC_NUMERIC_AVX_MODE) ||
						  bool(METRIC_NUMERIC_MIC_MODE) || bool(METRIC_NUMERIC_AVX512F_MODE)> {};

template <>
struct HasSIMDMultHelper<complex<float>, complex<float>>
	: public BoolConstant<(bool(METRIC_NUMERIC_SSE3_MODE) && !bool(METRIC_NUMERIC_MIC_MODE)) ||
						  (bool(METRIC_NUMERIC_AVX_MODE) && !bool(METRIC_NUMERIC_MIC_MODE)) ||
						  (bool(METRIC_NUMERIC_AVX512F_MODE) && !bool(METRIC_NUMERIC_MIC_MODE))> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <>
struct HasSIMDMultHelper<double, double>
	: public BoolConstant<bool(METRIC_NUMERIC_SSE2_MODE) || bool(METRIC_NUMERIC_AVX_MODE) ||
						  bool(METRIC_NUMERIC_MIC_MODE) || bool(METRIC_NUMERIC_AVX512F_MODE)> {};

template <>
struct HasSIMDMultHelper<complex<double>, complex<double>>
	: public BoolConstant<(bool(METRIC_NUMERIC_SSE3_MODE) && !bool(METRIC_NUMERIC_MIC_MODE)) ||
						  (bool(METRIC_NUMERIC_AVX_MODE) && !bool(METRIC_NUMERIC_MIC_MODE)) ||
						  (bool(METRIC_NUMERIC_AVX512F_MODE) && !bool(METRIC_NUMERIC_MIC_MODE))> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of a SIMD multiplication for the given data types.
// \ingroup math_type_traits
//
// Depending on the available instruction set (SSE, SSE2, SSE3, SSE4, AVX, AVX2, MIC, ...) and
// the used compiler, this type trait provides the information whether a SIMD multiplication
// operation exists for the two given data types \a T1 and \a T2 (ignoring the cv-qualifiers).
// In case the SIMD multiplication is available, the \a value member constant is set to \a true,
// the nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType. The following example assumes that AVX is available:

   \code
   mtrc::numeric::HasSIMDMult< int, int >::value        // Evaluates to 1
   mtrc::numeric::HasSIMDMult< double, double >::Type   // Results in TrueType
   mtrc::numeric::HasSIMDMult< complex<float>, float >  // Is derived from TrueType
   mtrc::numeric::HasSIMDMult< bool, bool >::value      // Evaluates to 0
   mtrc::numeric::HasSIMDMult< float, int >::Type       // Results in FalseType
   mtrc::numeric::HasSIMDMult< double, float >          // Is derived from FalseType
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename = void> // Restricting condition
struct HasSIMDMult : public BoolConstant<HasSIMDMultHelper<RemoveCVRef_t<T1>, RemoveCVRef_t<T2>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSIMDMult type trait.
// \ingroup math_type_traits
//
// The HasSIMDMult_v variable template provides a convenient shortcut to access the nested
// \a value of the HasSIMDMult class template. For instance, given the types \a T1 and \a T2
// the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSIMDMult<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HasSIMDMult_v<T1,T2>;
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
constexpr bool HasSIMDMult_v = HasSIMDMult<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
