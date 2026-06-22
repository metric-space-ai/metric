// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_DECLID_H
#define METRIC_NUMERIC_MATH_FUNCTORS_DECLID_H
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
/*!\brief Generic wrapper for the declid() function.
// \ingroup functors
*/
struct DeclId {
	//**********************************************************************************************
	/*!\brief Returns the result of the declid() function for the given object/value.
	//
	// \param a The given object/value.
	// \return The result of the declid() function for the given object/value.
	*/
	template <typename T>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(T &&a) const
	{
		return declid(std::forward<T>(a));
	}
	//**********************************************************************************************
};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
