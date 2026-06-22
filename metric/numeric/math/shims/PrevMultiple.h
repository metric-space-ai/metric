// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_PREVMULTIPLE_H
#define METRIC_NUMERIC_MATH_SHIMS_PREVMULTIPLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  PREVMULTIPLE SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Rounds down an integral value to the previous multiple of a given factor.
// \ingroup math
//
// \param value The integral value to be rounded down \f$[1..\infty)\f$.
// \param factor The factor of the multiple \f$[1..\infty)\f$.
// \return The previous multiple of the given factor.
//
// This function rounds down the given integral value to the previous multiple of the given
// integral factor. In case the integral value is already a multiple of the given factor, the
// value itself is returned. Note that the attempt to use the function with non-integral types
// results in a compilation error!
*/
template <typename T1, typename T2>
METRIC_NUMERIC_ALWAYS_INLINE constexpr auto prevMultiple(T1 value, T2 factor) noexcept
{
	return (value - (value % factor));
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
