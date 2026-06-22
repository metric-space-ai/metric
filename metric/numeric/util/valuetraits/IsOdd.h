// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_VALUETRAITS_ISODD_H
#define METRIC_NUMERIC_UTIL_VALUETRAITS_ISODD_H
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
/*!\brief Compile time check whether a compile time constant expression is odd.
// \ingroup value_traits
//
// This value trait tests whether the given integral value \a N is an odd value. In case the
// value is odd, the \a value member enumeration is set to 1, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set
// to 0, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsOdd<1>::value   // Evaluates to 1
   mtrc::numeric::IsOdd<3>::Type    // Results in TrueType
   mtrc::numeric::IsOdd<5>          // Is derived from TrueType
   mtrc::numeric::IsOdd<2>::value   // Evaluates to 0
   mtrc::numeric::IsOdd<4>::Type    // Results in FalseType
   mtrc::numeric::IsOdd<6>          // Is derived from FalseType
   \endcode
*/
template <size_t N> struct IsOdd : public BoolConstant<N % 2UL != 0UL> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsOdd value trait.
// \ingroup value_traits
//
// The IsOdd_v variable template provides a convenient shortcut to access the nested \a value
// of the IsOdd class template. For instance, given the compile time constant value \a N the
// following two statements are identical:

   \code
   constexpr bool value1 = IsOdd<N>::value;
   constexpr bool value2 = IsOdd_v<N>;
   \endcode
*/
template <size_t N> constexpr bool IsOdd_v = IsOdd<N>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
