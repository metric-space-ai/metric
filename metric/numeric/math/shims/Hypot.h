// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_HYPOT_H
#define METRIC_NUMERIC_MATH_SHIMS_HYPOT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>

namespace mtrc::numeric {

//=================================================================================================
//
//  HYPOT SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Import of the std::hypot() function into the Metric numeric namespace.
// \ingroup math_shims
*/
using std::hypot;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
