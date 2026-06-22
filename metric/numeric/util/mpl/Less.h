// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_MPL_LESS_H
#define METRIC_NUMERIC_UTIL_MPL_LESS_H
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
/*!\brief Compile time type comparison.
// \ingroup mpl
//
// The Less_t alias template compares the two given types using the less-than operator ('<'). In
// case \a T1::value is less than \a T2::value, the nested \a value member is set to 1. Otherwise
// it is set to 0.

   \code
   using namespace mtrc::numeric;

   Less_t< Int_t<2> , Int_t<5>  >::value    // Evaluates to true
   Less_t< Long_t<2>, Int_t<5>  >::value    // Evaluates to true
   Less_t< Int_t<2> , Long_t<2> >::value    // Evaluates to false
   Less_t< Long_t<5>, Int_t<2>  >::value    // Evaluates to false
   Less_t< Int_t<2>, Int_t<5> >::ValueType  // Results in bool
   \endcode
*/
template <typename T1 // The type of the left-hand side operand
		  ,
		  typename T2> // The type of the right-hand side operand
using Less_t = Bool_t<(T1::value < T2::value)>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Less_t alias.
// \ingroup mpl
//
// The Less_v variable template provides a convenient shortcut to access the nested \a value of
// the Less_t alias. For instance, given the types \a T1 and \a T2 the following two statements
// are identical:

   \code
   constexpr bool value1 = Less_t<T1,T2>::value;
   constexpr bool value2 = Less_v<T1,T2>;
   \endcode
*/
template <typename T1 // The type of the left-hand side operand
		  ,
		  typename T2> // The type of the right-hand side operand
constexpr bool Less_v = Less_t<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
