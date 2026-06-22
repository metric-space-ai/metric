// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDINVSQRT_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDINVSQRT_H
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
/*!\brief Auxiliary alias declaration for the HasSIMDInvSqrt type trait.
// \ingroup math_type_traits
*/
template <typename T> // Type of the operand
using HasSIMDInvSqrtHelper = BoolConstant<(IsFloat_v<T> || IsDouble_v<T>) && bool(METRIC_NUMERIC_SVML_MODE) &&
										  (bool(METRIC_NUMERIC_SSE_MODE) || bool(METRIC_NUMERIC_AVX_MODE) ||
										   bool(METRIC_NUMERIC_MIC_MODE) || bool(METRIC_NUMERIC_AVX512F_MODE))>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of a SIMD inverse square root operation for the given data type.
// \ingroup math_type_traits
//
// Depending on the available instruction set (SSE, SSE2, SSE3, SSE4, AVX, AVX2, MIC, ...) and
// the used compiler, this type trait provides the information whether a SIMD inverse square root
// operation exists for the given data type \a T (ignoring the cv-qualifiers). In case the SIMD
// operation is available, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType. The
// following example assumes that the Intel SVML is available:

   \code
   mtrc::numeric::HasSIMDInvSqrt< float >::value         // Evaluates to 1
   mtrc::numeric::HasSIMDInvSqrt< double >::Type         // Results in TrueType
   mtrc::numeric::HasSIMDInvSqrt< const double >         // Is derived from TrueType
   mtrc::numeric::HasSIMDInvSqrt< unsigned int >::value  // Evaluates to 0
   mtrc::numeric::HasSIMDInvSqrt< long double >::Type    // Results in FalseType
   mtrc::numeric::HasSIMDInvSqrt< complex<double> >      // Is derived from FalseType
   \endcode
*/
template <typename T> // Type of the operand
struct HasSIMDInvSqrt : public BoolConstant<HasSIMDInvSqrtHelper<RemoveCVRef_t<T>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSIMDInvSqrt type trait.
// \ingroup math_type_traits
//
// The HasSIMDInvSqrt_v variable template provides a convenient shortcut to access the nested
// \a value of the HasSIMDInvSqrt class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSIMDInvSqrt<T>::value;
   constexpr bool value2 = mtrc::numeric::HasSIMDInvSqrt_v<T>;
   \endcode
*/
template <typename T> // Type of the operand
constexpr bool HasSIMDInvSqrt_v = HasSIMDInvSqrt<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
