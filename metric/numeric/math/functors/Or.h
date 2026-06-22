// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_OR_H
#define METRIC_NUMERIC_MATH_FUNCTORS_OR_H
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
/*!\brief Generic wrapper for the logical OR operator.
// \ingroup functors
*/
struct Or {
	//**********************************************************************************************
	/*!\brief Returns the result of the logical OR operator for the given objects/values.
	//
	// \param a The left-hand side object/value.
	// \param b The right-hand side object/value.
	// \return The result of the logical OR operator for the given objects/values.
	*/
	template <typename T1, typename T2> METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) operator()(T1 &&a, T2 &&b) const
	{
		return std::forward<T1>(a) || std::forward<T2>(b);
	}
	//**********************************************************************************************
};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
