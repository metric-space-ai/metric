// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_TRUNC_H
#define METRIC_NUMERIC_MATH_SHIMS_TRUNC_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>

namespace mtrc::numeric {

//=================================================================================================
//
//  TRUNC SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Import of the std::trunc() function into the Metric numeric namespace.
// \ingroup math_shims
*/
using std::trunc;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
