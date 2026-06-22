// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_IMAGINARY_H
#define METRIC_NUMERIC_MATH_SHIMS_IMAGINARY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>

namespace mtrc::numeric {

//=================================================================================================
//
//  IMAG SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Import of the std::imag() function into the Metric numeric namespace.
// \ingroup math_shims
*/
using std::imag;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
