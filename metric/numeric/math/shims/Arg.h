// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_ARG_H
#define METRIC_NUMERIC_MATH_SHIMS_ARG_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Complex.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  ARG SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Import of the std::arg() function into the Metric numeric namespace.
// \ingroup math_shims
*/
using std::arg;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
