// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_MPL_XOR_H
#define METRIC_NUMERIC_UTIL_MPL_XOR_H
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
/*!\brief Compile time bitwise XOR evaluation.
// \ingroup mpl
//
// The Xor_t alias template performs at compile time a bitwise XOR evaluation of the two given
// compile time conditions:

   \code
   using namespace mtrc::numeric;

   using Type = int;

   Xor_t< IsSigned<Type>  , IsUnsigned<Type>      >::value  // Evaluates to 1
   Xor_t< IsIntegral<Type>, IsFloatingPoint<Type> >::value  // Evaluates to 1
   Xor_t< IsSigned<Type>  , IsIntegral<Type>      >::value  // Evaluates to 0
   Xor_t< IsUnsigned<Type>, IsFloatingPoint<Type> >::value  // Evaluates to 0
   \endcode
*/
template <typename T1 // Type of the first operand
		  ,
		  typename T2> // Type of the second operand
using Xor_t = Bool_t<(T1::value ^ T2::value)>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Xor_t alias.
// \ingroup mpl
//
// The Xor_v variable template provides a convenient shortcut to access the nested \a value of
// the Xor_t alias. For instance, given the types \a T1 and \a T2 the following two statements
// are identical:

   \code
   constexpr bool value1 = Xor_t<T1,T2>::value;
   constexpr bool value2 = Xor_v<T1,T2>;
   \endcode
*/
template <typename T1 // Type of the first operand
		  ,
		  typename T2> // Type of the second operand
constexpr bool Xor_v = Xor_t<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
