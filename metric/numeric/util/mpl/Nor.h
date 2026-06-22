// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_MPL_NOR_H
#define METRIC_NUMERIC_UTIL_MPL_NOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/mpl/Bools.h>
#include <metric/numeric/util/typetraits/IsSame.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time logical 'not or' evaluation.
// \ingroup mpl
//
// The Nor_t alias template performs at compile time a logical 'not or' evaluation of at least
// two compile time conditions:

   \code
   using namespace mtrc::numeric;

   using Type = int;

   Nor_t< IsFloat<Type>   , IsDouble<Type>        >::value  // Evaluates to 1
   Nor_t< IsIntegral<Type>, IsSigned<Type>        >::value  // Evaluates to 0
   Nor_t< IsIntegral<Type>, IsFloatingPoint<Type> >::value  // Evaluates to 0
   \endcode
*/
template <typename T1 // Type of the first mandatory operand
		  ,
		  typename T2 // Type of the second mandatory operand
		  ,
		  typename... Ts> // Types of the optional operands
using Nor_t = Bool_t<IsSame<Bools<false, T1::value, T2::value, (Ts::value)...>,
							Bools<T1::value, T2::value, (Ts::value)..., false>>::value>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Nor_t alias.
// \ingroup mpl
//
// The Nor_v variable template provides a convenient shortcut to access the nested \a value of
// the Nor_t alias. For instance, given the types \a T1 and \a T2 the following two statements
// are identical:

   \code
   constexpr bool value1 = Nor_t<T1,T2>::value;
   constexpr bool value2 = Nor_v<T1,T2>;
   \endcode
*/
template <typename T1 // Type of the first mandatory operand
		  ,
		  typename T2 // Type of the second mandatory operand
		  ,
		  typename... Ts> // Types of the optional operands
constexpr bool Nor_v = Nor_t<T1, T2, Ts...>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
