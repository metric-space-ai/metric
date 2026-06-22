// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_INVCBRT_H
#define METRIC_NUMERIC_MATH_SHIMS_INVCBRT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/shims/Cbrt.h>
#include <metric/numeric/math/shims/Invert.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/typetraits/IsBuiltin.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  INVCBRT SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the inverse cubic root of the given built-in value.
// \ingroup math_shims
//
// \param a The given built-in value \f$[0..\infty)\f$.
// \return The inverse cubic root of the given value.
//
// \note The given value must be in the range \f$[0..\infty)\f$. The validity of the value is
// only checked by an user assert.
*/
template <typename T, typename = EnableIf_t<IsBuiltin_v<T>>> inline auto invcbrt(T a) noexcept -> decltype(inv(cbrt(a)))
{
	METRIC_NUMERIC_USER_ASSERT(abs(a) != T(0), "Invalid built-in value detected");

	return inv(cbrt(a));
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
