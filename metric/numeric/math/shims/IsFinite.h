// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_ISFINITE_H
#define METRIC_NUMERIC_MATH_SHIMS_ISFINITE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/typetraits/IsArithmetic.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  ISFINITE SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Determines if the given floating point number has finite value.
// \ingroup math_shims
//
// \param a The floating point number to be checked.
// \return \a true in case the given floating point number is finite, \a false otherwise.
//
// This function determines if the given floating point number has finite value (i.e. it is
// normal, subnormal or zero, but not infinite or NaN).
*/
template <typename T, EnableIf_t<IsArithmetic_v<T>> * = nullptr> inline bool isfinite(T a) noexcept
{
	return std::isfinite(a);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
