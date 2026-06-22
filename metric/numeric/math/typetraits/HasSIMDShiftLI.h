// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDSHIFTLI_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDSHIFTLI_H
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
/*!\brief Auxiliary alias declaration for the HasSIMDShiftLI type trait.
// \ingroup math_type_traits
*/
template <typename T>
using HasSIMDShiftLIHelper =
	BoolConstant<(IsNumeric_v<T> && IsIntegral_v<T>) && ((bool(METRIC_NUMERIC_SSE2_MODE) && sizeof(T) >= 2UL) ||
														 (bool(METRIC_NUMERIC_AVX2_MODE) && sizeof(T) >= 2UL) ||
														 (bool(METRIC_NUMERIC_MIC_MODE) && sizeof(T) == 4UL) ||
														 (bool(METRIC_NUMERIC_AVX512BW_MODE) && sizeof(T) == 2UL) ||
														 (bool(METRIC_NUMERIC_AVX512F_MODE) && sizeof(T) >= 4UL))>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of a SIMD uniform left-shift for the given data types.
// \ingroup math_type_traits
//
// Depending on the available instruction set (SSE, SSE2, SSE3, SSE4, AVX, AVX2, MIC, ...),
// and the used compiler, this type trait provides the information whether a SIMD uniform
// left-shift operation exists for the two given data types \a T1 and \a T2 (ignoring the
// cv-qualifiers). In case the SIMD uniform left-shift is available, the \a value member
// constant is set to \a true, the nested type definition \a Type is \a TrueType, and the
// class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType. The following example assumes
// that AVX2 is available:

   \code
   mtrc::numeric::HasSIMDShiftLI< int >::value   // Evaluates to 1
   mtrc::numeric::HasSIMDShiftLI< short >::Type  // Results in TrueType
   mtrc::numeric::HasSIMDShiftLI< long >         // Is derived from TrueType
   mtrc::numeric::HasSIMDShiftLI< bool >::value  // Evaluates to 0
   mtrc::numeric::HasSIMDShiftLI< float >::Type  // Results in FalseType
   mtrc::numeric::HasSIMDShiftLI< double >       // Is derived from FalseType
   \endcode
*/
template <typename T> // Type of the operand
struct HasSIMDShiftLI : public BoolConstant<HasSIMDShiftLIHelper<RemoveCVRef_t<T>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSIMDShiftLI type trait.
// \ingroup math_type_traits
//
// The HasSIMDShiftLI_v variable template provides a convenient shortcut to access the nested
// \a value of the HasSIMDShiftLI class template. For instance, given the types \a T1 and
// \a T2 the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSIMDShiftLI<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HasSIMDShiftLI_v<T1,T2>;
   \endcode
*/
template <typename T> // Type of the operand
constexpr bool HasSIMDShiftLI_v = HasSIMDShiftLI<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
