// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_ERF_H
#define METRIC_NUMERIC_MATH_SHIMS_ERF_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>

namespace mtrc::numeric {

//=================================================================================================
//
//  ERF SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Import of the std::erf() function into the Metric numeric namespace.
// \ingroup math_shims
*/
using std::erf;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
