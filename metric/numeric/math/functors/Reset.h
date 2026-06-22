// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_RESET_H
#define METRIC_NUMERIC_MATH_FUNCTORS_RESET_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/shims/Reset.h>
#include <metric/numeric/system/HostDevice.h>
#include <metric/numeric/system/Inline.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Generic wrapper for the reset() function.
// \ingroup functors
*/
struct Reset {
	//**********************************************************************************************
	/*!\brief Returns the result of the reset() function for the given object/value.
	//
	// \param a The object/value to be resetted.
	// \return void
	*/
	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE void operator()(T &a) const
	{
		reset(a);
	}
	//**********************************************************************************************
};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
