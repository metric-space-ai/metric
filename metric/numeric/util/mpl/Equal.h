// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_MPL_EQUAL_H
#define METRIC_NUMERIC_UTIL_MPL_EQUAL_H
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
// The Equal_t alias template compares the two given types using the equality operator ('==').
// In case \a T1::value is equal to \a T2::value, the nested \a value member is set to \a true.
// Otherwise it is set to \a false.

   \code
   using namespace mtrc::numeric;

   Equal_t< Int_t<3>, Int_t<3>  >::value   // Evaluates to true
   Equal_t< Int_t<5>, Long_t<5> >::value   // Evaluates to true
   Equal_t< Long_t<0>, Int_t<4> >::value   // Evaluates to false
   Equal_t< Int_t<1>, Int_t<2>::ValueType  // Results in bool
   \endcode
*/
template <typename T1 // The type of the left-hand side operand
		  ,
		  typename T2> // The type of the right-hand side operand
using Equal_t = Bool_t<(T1::value == T2::value)>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Equal_t alias.
// \ingroup mpl
//
// The Equal_v variable template provides a convenient shortcut to access the nested \a value of
// the Equal_t alias. For instance, given the types \a T1 and \a T2 the following two statements
// are identical:

   \code
   constexpr bool value1 = Equal_t<T1,T2>::value;
   constexpr bool value2 = Equal_v<T1,T2>;
   \endcode
*/
template <typename T1 // The type of the left-hand side operand
		  ,
		  typename T2> // The type of the right-hand side operand
constexpr bool Equal_v = Equal_t<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
