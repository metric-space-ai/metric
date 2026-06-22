// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_LGAMMA_H
#define METRIC_NUMERIC_MATH_SHIMS_LGAMMA_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>

namespace mtrc::numeric {

//=================================================================================================
//
//  LGAMMA SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Import of the std::lgamma() function into the Metric numeric namespace.
// \ingroup math_shims
*/
using std::lgamma;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
