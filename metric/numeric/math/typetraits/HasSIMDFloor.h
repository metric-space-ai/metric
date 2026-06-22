// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDFLOOR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDFLOOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Vectorization.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsDouble.h>
#include <metric/numeric/util/typetraits/IsFloat.h>
#include <metric/numeric/util/typetraits/RemoveCVRef.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary alias declaration for the HasSIMDFloor type trait.
// \ingroup math_type_traits
*/
template <typename T> // Type of the operand
using HasSIMDFloorHelper = BoolConstant<(IsFloat_v<T> || IsDouble_v<T>) &&
										((!bool(METRIC_NUMERIC_MIC_MODE) && !bool(METRIC_NUMERIC_AVX512F_MODE) &&
										  (bool(METRIC_NUMERIC_SSE4_MODE) || bool(METRIC_NUMERIC_AVX_MODE))) ||
										 (bool(METRIC_NUMERIC_SVML_MODE) || bool(METRIC_NUMERIC_SLEEF_MODE) ||
										  bool(METRIC_NUMERIC_XSIMD_MODE)))>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of a SIMD floor operation for the given data type.
// \ingroup math_type_traits
//
// Depending on the available instruction set (SSE, SSE2, SSE3, SSE4, AVX, AVX2, MIC, ...) and
// the used compiler, this type trait provides the information whether a SIMD floor operation
// exists for the given data type \a T (ignoring the cv-qualifiers). In case the SIMD operation
// is available, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType. The following
// example assumes that AVX is available:

   \code
   mtrc::numeric::HasSIMDFloor< float >::value         // Evaluates to 1
   mtrc::numeric::HasSIMDFloor< double >::Type         // Results in TrueType
   mtrc::numeric::HasSIMDFloor< const double >         // Is derived from TrueType
   mtrc::numeric::HasSIMDFloor< unsigned int >::value  // Evaluates to 0
   mtrc::numeric::HasSIMDFloor< long double >::Type    // Results in FalseType
   mtrc::numeric::HasSIMDFloor< complex<double> >      // Is derived from FalseType
   \endcode
*/
template <typename T> // Type of the operand
struct HasSIMDFloor : public BoolConstant<HasSIMDFloorHelper<RemoveCVRef_t<T>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSIMDFloor type trait.
// \ingroup math_type_traits
//
// The HasSIMDFloor_v variable template provides a convenient shortcut to access the nested
// \a value of the HasSIMDFloor class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSIMDFloor<T>::value;
   constexpr bool value2 = mtrc::numeric::HasSIMDFloor_v<T>;
   \endcode
*/
template <typename T> // Type of the operand
constexpr bool HasSIMDFloor_v = HasSIMDFloor<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
