// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_MPL_GREATER_H
#define METRIC_NUMERIC_UTIL_MPL_GREATER_H
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
// The Greater_t alias template compares the two given types using the greater-than operator
// ('>'). In case \a T1::value is greater than \a T2::value, the nested \a value member is set
// to 1. Otherwise it is set to 0.

   \code
   using namespace mtrc::numeric;

   Greater_t< Int_t<5> , Int_t<2>  >::value      // Evaluates to true
   Greater_t< Int_t<5> , Long_t<2> >::value      // Evaluates to true
   Greater_t< Long_t<2>, Int_t<2>  >::value      // Evaluates to false
   Greater_t< Int_t<2> , Long_t<5> >::value      // Evaluates to false
   Greater_t< Int_t<5> , Int_t<2>  >::ValueType  // Results in bool
   \endcode
*/
template <typename T1 // The type of the left-hand side operand
		  ,
		  typename T2> // The type of the right-hand side operand
using Greater_t = Bool_t<(T1::value > T2::value)>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Greater_t alias.
// \ingroup mpl
//
// The Greater_v variable template provides a convenient shortcut to access the nested \a value of
// the Greater_t alias. For instance, given the types \a T1 and \a T2 the following two statements
// are identical:

   \code
   constexpr bool value1 = Greater_t<T1,T2>::value;
   constexpr bool value2 = Greater_v<T1,T2>;
   \endcode
*/
template <typename T1 // The type of the left-hand side operand
		  ,
		  typename T2> // The type of the right-hand side operand
constexpr bool Greater_v = Greater_t<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
