// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDLOG1P_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDLOG1P_H
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
/*!\brief Auxiliary alias declaration for the HasSIMDLog1p type trait.
// \ingroup math_type_traits
*/
template <typename T> // Type of the operand
using HasSIMDLog1pHelper = BoolConstant<(IsFloat_v<T> || IsDouble_v<T>) &&
										(bool(METRIC_NUMERIC_SLEEF_MODE) || bool(METRIC_NUMERIC_XSIMD_MODE)) &&
										(bool(METRIC_NUMERIC_SSE_MODE) || bool(METRIC_NUMERIC_AVX_MODE) ||
										 bool(METRIC_NUMERIC_MIC_MODE) || bool(METRIC_NUMERIC_AVX512F_MODE))>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of a SIMD log1p operation for the given data type.
// \ingroup math_type_traits
//
// Depending on the available instruction set (SSE, SSE2, SSE3, SSE4, AVX, AVX2, MIC, ...) and
// the used compiler, this type trait provides the information whether a SIMD log1p operation
// exists for the given data type \a T (ignoring the cv-qualifiers). In case the SIMD operation
// is available, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set
// to \a false, \a Type is \a FalseType, and the class derives from \a FalseType. The following
// example assumes that the Intel SVML is available:

   \code
   mtrc::numeric::HasSIMDLog1p< float >::value         // Evaluates to 1
   mtrc::numeric::HasSIMDLog1p< double >::Type         // Results in TrueType
   mtrc::numeric::HasSIMDLog1p< const double >         // Is derived from TrueType
   mtrc::numeric::HasSIMDLog1p< unsigned int >::value  // Evaluates to 0
   mtrc::numeric::HasSIMDLog1p< long double >::Type    // Results in FalseType
   mtrc::numeric::HasSIMDLog1p< complex<double> >      // Is derived from FalseType
   \endcode
*/
template <typename T> // Type of the operand
struct HasSIMDLog1p : public BoolConstant<HasSIMDLog1pHelper<RemoveCVRef_t<T>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSIMDLog1p type trait.
// \ingroup math_type_traits
//
// The HasSIMDLog1p_v variable template provides a convenient shortcut to access the nested
// \a value of the HasSIMDLog1p class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSIMDLog1p<T>::value;
   constexpr bool value2 = mtrc::numeric::HasSIMDLog1p_v<T>;
   \endcode
*/
template <typename T> // Type of the operand
constexpr bool HasSIMDLog1p_v = HasSIMDLog1p<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
