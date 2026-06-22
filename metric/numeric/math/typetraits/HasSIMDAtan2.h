// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDATAN2_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDATAN2_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Vectorization.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsDouble.h>
#include <metric/numeric/util/typetraits/IsFloat.h>
#include <metric/numeric/util/typetraits/IsSame.h>
#include <metric/numeric/util/typetraits/RemoveCVRef.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary alias declaration for the HasSIMDAtan2 type trait.
// \ingroup math_type_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
using HasSIMDAtan2Helper = BoolConstant<IsSame_v<T1, T2> && (IsFloat_v<T1> || IsDouble_v<T1>) &&
										(bool(METRIC_NUMERIC_SVML_MODE) || bool(METRIC_NUMERIC_SLEEF_MODE) ||
										 bool(METRIC_NUMERIC_XSIMD_MODE)) &&
										(bool(METRIC_NUMERIC_SSE_MODE) || bool(METRIC_NUMERIC_AVX_MODE) ||
										 bool(METRIC_NUMERIC_MIC_MODE) || bool(METRIC_NUMERIC_AVX512F_MODE))>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of a SIMD inverse tangent operation for the given data types.
// \ingroup math_type_traits
//
// Depending on the available instruction set (SSE, SSE2, SSE3, SSE4, AVX, AVX2, MIC, ...) and
// the used compiler, this type trait provides the information whether a SIMD multi-valued
// inverse tangent operation exists for the given data types \a T1 and \a T2 (ignoring the
// cv-qualifiers). In case the SIMD operation is available, the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives
// from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the
// class derives from \a FalseType. The following example assumes that the Intel SVML is
// available:

   \code
   mtrc::numeric::HasSIMDAtan2< float, float >::value               // Evaluates to 1
   mtrc::numeric::HasSIMDAtan2< double, double >::Type              // Results in TrueType
   mtrc::numeric::HasSIMDAtan2< const double, volatile double >     // Is derived from TrueType
   mtrc::numeric::HasSIMDAtan2< int, int >::value                   // Evaluates to 0
   mtrc::numeric::HasSIMDAtan2< long double, long double >::Type    // Results in FalseType
   mtrc::numeric::HasSIMDAtan2< complex<double>, complex<double> >  // Is derived from FalseType
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
struct HasSIMDAtan2 : public BoolConstant<HasSIMDAtan2Helper<RemoveCVRef_t<T1>, RemoveCVRef_t<T2>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSIMDAtan2 type trait.
// \ingroup math_type_traits
//
// The HasSIMDAtan2_v variable template provides a convenient shortcut to access the nested
// \a value of the HasSIMDAtan2 class template. For instance, given the types \a T1 and \a T2
// the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSIMDAtan2<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HasSIMDAtan2_v<T1,T2>;
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
constexpr bool HasSIMDAtan2_v = HasSIMDAtan2<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
