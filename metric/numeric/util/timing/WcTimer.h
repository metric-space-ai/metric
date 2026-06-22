// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TIMING_WCTIMER_H
#define METRIC_NUMERIC_UTIL_TIMING_WCTIMER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/timing/Timer.h>
#include <metric/numeric/util/timing/WcPolicy.h>

namespace mtrc::numeric {

namespace timing {

//=================================================================================================
//
//  TYPE DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Progress timer for wall clock time measurements.
// \ingroup timing
//
// The WcTimer combines the Timer class template with the WcPolicy timing policy. It measures
// the amount of "wall clock" time elapsing for the processing of a programm or code fragment.
// In contrast to the measurement of CPU time, the wall clock time also contains waiting times
// such as input/output operations.
*/
using WcTimer = Timer<WcPolicy>;
//*************************************************************************************************

} // namespace timing

} // namespace mtrc::numeric

#endif
