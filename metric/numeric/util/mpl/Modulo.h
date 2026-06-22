// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_MPL_MODULO_H
#define METRIC_NUMERIC_UTIL_MPL_MODULO_H
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
/*!\brief Compile time integral modulo operation.
// \ingroup mpl
//
// The Modulo_t alias template returns the result of a modulo operation between the two given
// template arguments \a T1 and \a T2. In order for Modulo_t to be able to perform the operation
// with the given two types, both arguments are required to have a nested member \a value. The
// result of the modulo operation can be accessed via the nested member \a value, the resulting
// type is available via the nested type \a ValueType.

   \code
   mtrc::numeric::Modulo_t< Int_t<3> , Int_t<2>  >::value      // Results in 1
   mtrc::numeric::Modulo_t< Long_t<3>, Int_t<2>  >::ValueType  // Results in long
   mtrc::numeric::Modulo_t< Int_t<3> , Long_t<2> >::ValueType  // Results in long
   \endcode
*/
template <typename T1 // Type of the first compile time value
		  ,
		  typename T2> // Type of the second compile time value
using Modulo_t =
	IntegralConstant<CommonType_t<typename T1::ValueType, typename T2::ValueType>, (T1::value % T2::value)>;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
