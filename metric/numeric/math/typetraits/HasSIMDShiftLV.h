// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDSHIFTLV_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDSHIFTLV_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Vectorization.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsIntegral.h>
#include <metric/numeric/util/typetraits/IsNumeric.h>
#include <metric/numeric/util/typetraits/IsSigned.h>
#include <metric/numeric/util/typetraits/RemoveCVRef.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary alias declaration for the HasSIMDShiftLV type trait.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
using HasSIMDShiftLVHelper = BoolConstant<
	(IsNumeric_v<T1> && IsIntegral_v<T1> && IsNumeric_v<T2> && IsIntegral_v<T2> && sizeof(T1) == sizeof(T2)) &&
	((bool(METRIC_NUMERIC_AVX2_MODE) && sizeof(T1) >= 4UL) || (bool(METRIC_NUMERIC_MIC_MODE) && sizeof(T1) == 4UL) ||
	 (bool(METRIC_NUMERIC_AVX512BW_MODE) && sizeof(T1) == 2UL) ||
	 (bool(METRIC_NUMERIC_AVX512F_MODE) && sizeof(T1) >= 4UL))>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of an SIMD elementwise left-shift for the given data types.
// \ingroup math_type_traits
//
// Depending on the available instruction set (SSE, SSE2, SSE3, SSE4, AVX, AVX2, MIC, ...),
// and the used compiler, this type trait provides the information whether an SIMD elementwise
// left-shift operation exists for the two given data types \a T1 and \a T2 (ignoring the
// cv-qualifiers). In case the SIMD elementwise left-shift is available, the \a value member
// constant is set to \a true, the nested type definition \a Type is \a TrueType, and the class
// derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType,
// and the class derives from \a FalseType. The following example assumes that AVX2 is available:

   \code
   mtrc::numeric::HasSIMDShiftLV< int, int >::value     // Evaluates to 1
   mtrc::numeric::HasSIMDShiftLV< long, long >::Type    // Results in TrueType
   mtrc::numeric::HasSIMDShiftLV< unsigned, unsigned >  // Is derived from TrueType
   mtrc::numeric::HasSIMDShiftLV< bool, bool >::value   // Evaluates to 0
   mtrc::numeric::HasSIMDShiftLV< float, float >::Type  // Results in FalseType
   mtrc::numeric::HasSIMDShiftLV< double, double >      // Is derived from FalseType
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
struct HasSIMDShiftLV : public BoolConstant<HasSIMDShiftLVHelper<RemoveCVRef_t<T1>, RemoveCVRef_t<T2>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSIMDShiftLV type trait.
// \ingroup math_type_traits
//
// The HasSIMDShiftLV_v variable template provides a convenient shortcut to access the nested
// \a value of the HasSIMDShiftLV class template. For instance, given the types \a T1 and
// \a T2 the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSIMDShiftLV<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HasSIMDShiftLV_v<T1,T2>;
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
constexpr bool HasSIMDShiftLV_v = HasSIMDShiftLV<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
