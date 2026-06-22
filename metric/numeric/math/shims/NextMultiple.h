// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_NEXTMULTIPLE_H
#define METRIC_NUMERIC_MATH_SHIMS_NEXTMULTIPLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  NEXTMULTIPLE SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Rounds up an integral value to the next multiple of a given factor.
// \ingroup math
//
// \param value The integral value to be rounded up \f$[1..\infty)\f$.
// \param factor The factor of the multiple \f$[1..\infty)\f$.
// \return The next multiple of the given factor.
//
// This function rounds up the given integral value to the next multiple of the given integral
// factor. In case the integral value is already a multiple of the given factor, the value itself
// is returned. Note that the attempt to use the function with non-integral types results in a
// compilation error!
*/
template <typename T1, typename T2>
METRIC_NUMERIC_ALWAYS_INLINE constexpr auto nextMultiple(T1 value, T2 factor) noexcept
{
	return (value + (factor - (value % factor)) % factor);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
