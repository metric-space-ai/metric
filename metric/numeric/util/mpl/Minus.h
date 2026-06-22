// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_MPL_MINUS_H
#define METRIC_NUMERIC_UTIL_MPL_MINUS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/CommonType.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time integral subtraction.
// \ingroup mpl
//
// The Minus_t alias template returns the difference of the two given template arguments \a T1
// and \a T2. In order for Minus_t to be able to subtract the two types, both arguments are
// required to have a nested member \a value. The result of the subtraction can be accessed
// via the nested member \a value, the resulting type is available via the nested type
// \a ValueType.

   \code
   mtrc::numeric::Minus_t< Int_t<3> , Int_t<2>  >::value      // Results in 5
   mtrc::numeric::Minus_t< Long_t<3>, Int_t<2>  >::ValueType  // Results in long
   mtrc::numeric::Minus_t< Int_t<3> , Long_t<2> >::ValueType  // Results in long
   \endcode
*/
template <typename T1 // Type of the first compile time value
		  ,
		  typename T2> // Type of the second compile time value
using Minus_t = IntegralConstant<CommonType_t<typename T1::ValueType, typename T2::ValueType>, (T1::value - T2::value)>;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
