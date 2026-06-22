// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_NOT_H
#define METRIC_NUMERIC_MATH_FUNCTORS_NOT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Generic wrapper for the logical NOT operator.
// \ingroup functors
*/
struct Not {
	//**********************************************************************************************
	/*!\brief Returns the result of the logical NOT operator for the given object/value.
	//
	// \param a The given object/value.
	// \return The result of the logical NOT operator for the given object/value.
	*/
	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) operator()(T &&a) const
	{
		return !std::forward<T>(a);
	}
	//**********************************************************************************************
};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
