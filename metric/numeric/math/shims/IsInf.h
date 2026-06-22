// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_ISINF_H
#define METRIC_NUMERIC_MATH_SHIMS_ISINF_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/typetraits/IsArithmetic.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  ISINF SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Determines if the given floating point number is a positive or negative infinity.
// \ingroup math_shims
//
// \param a The floating point number to be checked.
// \return \a true in case the given floating point number is infinity, \a false otherwise.
*/
template <typename T, EnableIf_t<IsArithmetic_v<T>> * = nullptr> inline bool isinf(T a) noexcept
{
	return std::isinf(a);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
