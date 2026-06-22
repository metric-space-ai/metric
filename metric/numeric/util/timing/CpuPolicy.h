// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TIMING_CPUPOLICY_H
#define METRIC_NUMERIC_UTIL_TIMING_CPUPOLICY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Time.h>

namespace mtrc::numeric {

namespace timing {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Timing policy for the measurement of the CPU time.
// \ingroup timing
//
// The CpuPolicy class represents the timing policy for CPU time measurements that can be used
// in combination with the Timer class template. This combination is realized with the CpuTimer
// type definition.
*/
struct CpuPolicy {
  public:
	//**Timing functions****************************************************************************
	/*!\name Timing functions */
	//@{
	static inline double getTimestamp();
	//@}
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  TIMING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns a timestamp of the current CPU time in seconds.
//
// \return CPU timestamp in seconds.
*/
inline double CpuPolicy::getTimestamp() { return getCpuTime(); }
//*************************************************************************************************

} // namespace timing

} // namespace mtrc::numeric

#endif
