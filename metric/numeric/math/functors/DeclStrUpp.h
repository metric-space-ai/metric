// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_DECLSTRUPP_H
#define METRIC_NUMERIC_MATH_FUNCTORS_DECLSTRUPP_H
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
/*!\brief Generic wrapper for the declstrupp() function.
// \ingroup functors
*/
struct DeclStrUpp {
	//**********************************************************************************************
	/*!\brief Returns the result of the declstrupp() function for the given object/value.
	//
	// \param a The given object/value.
	// \return The result of the declstrupp() function for the given object/value.
	*/
	template <typename T>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(T &&a) const
	{
		return declstrupp(std::forward<T>(a));
	}
	//**********************************************************************************************
};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
