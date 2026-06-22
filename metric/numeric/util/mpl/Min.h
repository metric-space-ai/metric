// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_MPL_MIN_H
#define METRIC_NUMERIC_UTIL_MPL_MIN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/mpl/Less.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time value evaluation.
// \ingroup mpl
//
// The Min_t alias template selects the smaller of the two given template arguments \a T1
// and \a T2. In order for Min_t to be able to determine the smaller type, both arguments
// are required to have a nested member \a value. The result of the minimum operation can be
// accessed via the nested member \a value, the resulting type is available via the nested
// type \a ValueType.

   \code
   mtrc::numeric::Min_t< Int_t<3> , Int_t<2>  >::value      // Results in 2
   mtrc::numeric::Min_t< Long_t<3>, Int_t<2>  >::ValueType  // Results in int
   mtrc::numeric::Min_t< Int_t<3> , Long_t<2> >::ValueType  // Results in long
   \endcode
*/
template <typename T1 // Type of the first compile time value
		  ,
		  typename T2> // Type of the second compile time value
using Min_t = If_t<Less_t<T1, T2>::value, T1, T2>;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
