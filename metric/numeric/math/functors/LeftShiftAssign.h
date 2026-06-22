// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_LEFTSHIFTASSIGN_H
#define METRIC_NUMERIC_MATH_FUNCTORS_LEFTSHIFTASSIGN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Generic wrapper for bitwise left-shift assignment.
// \ingroup functors
*/
struct LeftShiftAssign {
	//**********************************************************************************************
	/*!\brief Performs a bitwise left-shift assignment with the given objects/values.
	//
	// \param a The target left-hand side object/value.
	// \param b The right-hand side object/value for the bitwise left-shift operation.
	// \return void
	*/
	template <typename T1, typename T2> METRIC_NUMERIC_ALWAYS_INLINE void operator()(T1 &a, const T2 &b) const
	{
		a <<= b;
	}
	//**********************************************************************************************
};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
