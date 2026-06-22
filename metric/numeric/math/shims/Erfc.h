// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_ERFC_H
#define METRIC_NUMERIC_MATH_SHIMS_ERFC_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>

namespace mtrc::numeric {

//=================================================================================================
//
//  ERFC SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Import of the std::erfc() function into the Metric numeric namespace.
// \ingroup math_shims
*/
using std::erfc;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
