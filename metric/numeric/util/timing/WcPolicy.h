// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TIMING_WCPOLICY_H
#define METRIC_NUMERIC_UTIL_TIMING_WCPOLICY_H
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
/*!\brief Timing policy for the measurement of the wall clock time.
// \ingroup timing
//
// The WcPolicy class represents the timing policy for wall clock time measurements that can be
// used in combination with the Timer class template. This combination is realized with the WcTimer
// type definition.
*/
struct WcPolicy {
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
/*!\brief Returns a timestamp of the current wall clock time in seconds.
//
// \return Wall clock timestamp in seconds.
*/
inline double WcPolicy::getTimestamp() { return getWcTime(); }
//*************************************************************************************************

} // namespace timing

} // namespace mtrc::numeric

#endif
