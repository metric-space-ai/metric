// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_INVSQRT_H
#define METRIC_NUMERIC_MATH_SHIMS_INVSQRT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/shims/Invert.h>
#include <metric/numeric/math/shims/Sqrt.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/typetraits/IsBuiltin.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  INVSQRT SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the inverse square root of the given built-in value.
// \ingroup math_shims
//
// \param a The given built-in value \f$[0..\infty)\f$.
// \return The inverse square root of the given value.
//
// \note The given value must be in the range \f$[0..\infty)\f$. The validity of the value is
// only checked by an user assert.
*/
template <typename T, typename = EnableIf_t<IsBuiltin_v<T>>> inline auto invsqrt(T a) noexcept -> decltype(inv(sqrt(a)))
{
	METRIC_NUMERIC_USER_ASSERT(a > T(0), "Invalid built-in value detected");

	return inv(sqrt(a));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the inverse square root of the given complex number.
// \ingroup math_shims
//
// \param a The given complex number.
// \return The inverse square root of the given complex number.
//
// \note The given complex number must not be zero. The validity of the value is only checked by
// an user assert.
*/
template <typename T, typename = EnableIf_t<IsBuiltin_v<T>>>
inline auto invsqrt(const complex<T> &a) noexcept -> decltype(inv(sqrt(a)))
{
	METRIC_NUMERIC_USER_ASSERT(abs(a) != T(0), "Invalid complex value detected");

	return inv(sqrt(a));
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
