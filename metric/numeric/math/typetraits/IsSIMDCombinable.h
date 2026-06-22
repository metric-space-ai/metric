// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSIMDCOMBINABLE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSIMDCOMBINABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsIntegral.h>
#include <metric/numeric/util/typetraits/IsNumeric.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the IsSIMDCombinable type trait.
// \ingroup math_type_traits
*/
template <typename T1, typename T2, typename... Ts>
struct IsSIMDCombinableHelper
	: public BoolConstant<IsSIMDCombinableHelper<T1, T2>::value && IsSIMDCombinableHelper<T2, Ts...>::value> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsSIMDCombinableHelper class template for two matching types.
template <typename T> struct IsSIMDCombinableHelper<T, T> : public BoolConstant<IsNumeric_v<T>> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsSIMDCombinableHelper class template for two different types.
template <typename T1, typename T2>
struct IsSIMDCombinableHelper<T1, T2> : public BoolConstant<IsNumeric_v<T1> && IsIntegral_v<T1> && IsNumeric_v<T2> &&
															IsIntegral_v<T2> && sizeof(T1) == sizeof(T2)> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for data types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given types can be combined in the context of SIMD
// operations. By default, all numeric data types are considered combinable to themselves. Also,
// different integral types of the same size can be combined. If the types are determined to be
// SIMD combinable, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsSIMDCombinable< int, unsigned int >::value        // Evaluates to 1
   mtrc::numeric::IsSIMDCombinable< double, double >::Type            // Results in TrueType
   mtrc::numeric::IsSIMDCombinable< complex<float>, complex<float> >  // Is derived from TrueType
   mtrc::numeric::IsSIMDCombinable< int, float >::value               // Evaluates to 0
   mtrc::numeric::IsSIMDCombinable< double, float >::Type             // Results in FalseType
   mtrc::numeric::IsSIMDCombinable< complex<int>, complex<float> >    // Is derived from FalseType
   \endcode
*/
template <typename T1, typename T2, typename... Ts>
struct IsSIMDCombinable : public BoolConstant<IsSIMDCombinableHelper<T1, T2, Ts...>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSIMDCombinable type trait.
// \ingroup math_type_traits
//
// The IsSIMDCombinable_v variable template provides a convenient shortcut to access the nested
// \a value of the IsSIMDCombinable class template. For instance, given the types \a T1 and
// \a T2 the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSIMDCombinable<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::IsSIMDCombinable_v<T1,T2>;
   \endcode
*/
template <typename T1, typename T2, typename... Ts>
constexpr bool IsSIMDCombinable_v = IsSIMDCombinable<T1, T2, Ts...>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
