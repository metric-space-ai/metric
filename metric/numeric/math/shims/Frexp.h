// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_FREXP_H
#define METRIC_NUMERIC_MATH_SHIMS_FREXP_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>

namespace mtrc::numeric {

//=================================================================================================
//
//  FREXP SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Import of the std::frexp() function into the Metric numeric namespace.
// \ingroup math_shims
*/
using std::frexp;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
