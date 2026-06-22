// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_MPL_NOT_H
#define METRIC_NUMERIC_UTIL_MPL_NOT_H
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
/*!\brief Compile time type negation.
// \ingroup mpl
//
// The Not_t alias template negates the given compile time condition. In case the given condition
// would evaluate to \a true, the nested member enumeration is set to \a false and vice versa:

   \code
   using namespace mtrc::numeric;

   Not_t< IsIntegral<int> >::value    // Evaluates to false
   Not_t< IsDouble<int>   >::value    // Evaluates to true
   Not_t< IsSigned<int> >::ValueType  // Results in bool
   \endcode
*/
template <typename C> // Condition to be negated
using Not_t = Bool_t<!C::value>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Not_t alias.
// \ingroup mpl
//
// The Not_v variable template provides a convenient shortcut to access the nested \a value of
// the Not_t alias. For instance, given the type \a C the following two statements are identical:

   \code
   constexpr bool value1 = Not_t<C>::value;
   constexpr bool value2 = Not_v<C>;
   \endcode
*/
template <typename C> // Condition to be negated
constexpr bool Not_v = Not_t<C>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
