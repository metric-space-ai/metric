// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TIMING_CPUTIMER_H
#define METRIC_NUMERIC_UTIL_TIMING_CPUTIMER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/timing/CpuPolicy.h>
#include <metric/numeric/util/timing/Timer.h>

namespace mtrc::numeric {

namespace timing {

//=================================================================================================
//
//  TYPE DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Progress timer for CPU time measurements.
// \ingroup timing
//
// The CpuTimer combines the Timer class template with the CpuPolicy timing policy. It measures
// the amount of time the measured program or code fragment uses in processing central processing
// unit (CPU) instructions.
*/
using CpuTimer = Timer<CpuPolicy>;
//*************************************************************************************************

} // namespace timing

} // namespace mtrc::numeric

#endif
