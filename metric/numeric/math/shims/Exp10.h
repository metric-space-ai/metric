// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_EXP10_H
#define METRIC_NUMERIC_MATH_SHIMS_EXP10_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/shims/Pow.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/typetraits/IsBuiltin.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  EXP10 SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the base-10 exponent of the given built-in value.
// \ingroup math_shims
//
// \param a The given built-in value.
// \return The base-10 exponent of the given value.
*/
template <typename T, typename = EnableIf_t<IsBuiltin_v<T>>>
METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) exp10(const T &a) noexcept(noexcept(pow(T(10), a)))
{
	return pow(T(10), a);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
