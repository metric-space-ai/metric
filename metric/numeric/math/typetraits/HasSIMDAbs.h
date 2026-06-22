// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDABS_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDABS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Compiler.h>
#include <metric/numeric/system/Vectorization.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsDouble.h>
#include <metric/numeric/util/typetraits/IsFloat.h>
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
/*!\brief Auxiliary alias declaration for the HasSIMDAbs type trait.
// \ingroup math_type_traits
*/
template <typename T> // Type of the operand
using HasSIMDAbsHelper = BoolConstant<
	((IsNumeric_v<T> && IsIntegral_v<T> && IsSigned_v<T>) &&
	 ((bool(METRIC_NUMERIC_SSSE3_MODE) && sizeof(T) <= 4UL) || (bool(METRIC_NUMERIC_AVX2_MODE) && sizeof(T) <= 4UL) ||
	  (bool(METRIC_NUMERIC_MIC_MODE) && sizeof(T) >= 4UL) || (bool(METRIC_NUMERIC_AVX512BW_MODE) && sizeof(T) <= 2UL) ||
	  (bool(METRIC_NUMERIC_AVX512F_MODE) && sizeof(T) >= 4UL))) ||
	(IsFloat_v<T> && (bool(METRIC_NUMERIC_SSE2_MODE) || bool(METRIC_NUMERIC_AVX_MODE) ||
					  bool(METRIC_NUMERIC_MIC_MODE) || bool(METRIC_NUMERIC_AVX512F_MODE))) ||
	(IsDouble_v<T> &&
	 !(bool(METRIC_NUMERIC_GNU_COMPILER) && (bool(METRIC_NUMERIC_MIC_MODE) || bool(METRIC_NUMERIC_AVX512F_MODE))) &&
	 (bool(METRIC_NUMERIC_SSE2_MODE) || bool(METRIC_NUMERIC_AVX_MODE) || bool(METRIC_NUMERIC_MIC_MODE) ||
	  bool(METRIC_NUMERIC_AVX512F_MODE)))>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of a SIMD absolute value operation for the given data type.
// \ingroup math_type_traits
//
// Depending on the available instruction set (SSE, SSE2, SSE3, SSE4, AVX, AVX2, MIC, ...) and
// the used compiler, this type trait provides the information whether a SIMD absolute value
// operation exists for the given data type \a T (ignoring the cv-qualifiers). In case the SIMD
// operation is available, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType. The
// following example assumes that AVX is available:

   \code
   mtrc::numeric::HasSIMDAbs< char >::value          // Evaluates to 1
   mtrc::numeric::HasSIMDAbs< short >::Type          // Results in TrueType
   mtrc::numeric::HasSIMDAbs< int >                  // Is derived from TrueType
   mtrc::numeric::HasSIMDAbs< unsigned int >::value  // Evaluates to 0
   mtrc::numeric::HasSIMDAbs< long double >::Type    // Results in FalseType
   mtrc::numeric::HasSIMDAbs< complex<int> >         // Is derived from FalseType
   \endcode
*/
template <typename T> // Type of the operand
struct HasSIMDAbs : public BoolConstant<HasSIMDAbsHelper<RemoveCVRef_t<T>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSIMDAbs type trait.
// \ingroup math_type_traits
//
// The HasSIMDAbs_v variable template provides a convenient shortcut to access the nested
// \a value of the HasSIMDAbs class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSIMDAbs<T>::value;
   constexpr bool value2 = mtrc::numeric::HasSIMDAbs_v<T>;
   \endcode
*/
template <typename T> // Type of the operand
constexpr bool HasSIMDAbs_v = HasSIMDAbs<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
