// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_CLEAR_H
#define METRIC_NUMERIC_MATH_FUNCTORS_CLEAR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/shims/Clear.h>
#include <metric/numeric/system/HostDevice.h>
#include <metric/numeric/system/Inline.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Generic wrapper for the clear() function.
// \ingroup functors
*/
struct Clear {
	//**********************************************************************************************
	/*!\brief Returns the result of the clear() function for the given object/value.
	//
	// \param a The object/value to be cleared.
	// \return void
	*/
	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE void operator()(T &a) const
	{
		clear(a);
	}
	//**********************************************************************************************
};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
