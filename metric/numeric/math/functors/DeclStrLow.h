// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_DECLSTRLOW_H
#define METRIC_NUMERIC_MATH_FUNCTORS_DECLSTRLOW_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/HostDevice.h>
#include <metric/numeric/system/Inline.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Generic wrapper for the declstrlow() function.
// \ingroup functors
*/
struct DeclStrLow {
	//**********************************************************************************************
	/*!\brief Returns the result of the declstrlow() function for the given object/value.
	//
	// \param a The given object/value.
	// \return The result of the declstrlow() function for the given object/value.
	*/
	template <typename T>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(T &&a) const
	{
		return declstrlow(std::forward<T>(a));
	}
	//**********************************************************************************************
};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
