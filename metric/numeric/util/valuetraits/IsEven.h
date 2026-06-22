// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_VALUETRAITS_ISEVEN_H
#define METRIC_NUMERIC_UTIL_VALUETRAITS_ISEVEN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check whether a compile time constant expression is even.
// \ingroup value_traits
//
// This value trait tests whether the given integral value \a N is an even value. In case the
// value is even, the \a value member enumeration is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set
// to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsEven<2>::value   // Evaluates to 1
   mtrc::numeric::IsEven<4>::Type    // Results in TrueType
   mtrc::numeric::IsEven<6>          // Is derived from TrueType
   mtrc::numeric::IsEven<1>::value   // Evaluates to 0
   mtrc::numeric::IsEven<3>::Type    // Results in FalseType
   mtrc::numeric::IsEven<5>          // Is derived from FalseType
   \endcode
*/
template <size_t N> struct IsEven : public BoolConstant<N % 2UL == 0UL> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsEven value trait.
// \ingroup value_traits
//
// The IsEven_v variable template provides a convenient shortcut to access the nested \a value
// of the IsEven class template. For instance, given the compile time constant value \a N the
// following two statements are identical:

   \code
   constexpr bool value1 = IsEven<N>::value;
   constexpr bool value2 = IsEven_v<N>;
   \endcode
*/
template <size_t N> constexpr bool IsEven_v = IsEven<N>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
