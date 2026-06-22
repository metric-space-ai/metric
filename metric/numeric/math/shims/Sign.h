// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_SIGN_H
#define METRIC_NUMERIC_MATH_SHIMS_SIGN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/typetraits/IsIntegral.h>
#include <metric/numeric/util/typetraits/IsSigned.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  SIGN SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Evaluating the sign of the given value.
// \ingroup math_shims
//
// \param a The given value.
// \return 1 if the value is greater than zero, 0 if it is zero, and -1 if it is less than zero.
//
// The sign function evaluates the sign of the given value \a a of the built-in data type \a T.
// It returns 1 if \a a is greater than zero, 0 if \a a is zero, and -1 if \a a is less than zero.
*/
template <typename T, typename = EnableIf_t<IsIntegral_v<T>>> constexpr T sign(T a) noexcept
{
	return (IsSigned_v<T>) ? (T(0) < a) - (a < T(0)) : (T(0) < a);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Evaluating the sign of the given single precision value.
// \ingroup math_shims
//
// \param a The given single precision value.
// \return 1 if the value is greater than zero, 0 if it is zero, and -1 if it is less than zero.
//
// The sign function evaluates the sign of the given single precision value \a a. It returns 1.0
// if \a a is greater than zero, 0.0 if \a a is zero, -1.0 if \a a is less than zero, and \c NaN
// if a is \c NaN.
*/
constexpr float sign(float a) noexcept
{
	if (0.0F < a)
		return 1.0F;
	else if (a < 0.0F)
		return -1.0F;
	else
		return a;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Evaluating the sign of the given double precision value.
// \ingroup math_shims
//
// \param a The given double precision value.
// \return 1 if the value is greater than zero, 0 if it is zero, and -1 if it is less than zero.
//
// The sign function evaluates the sign of the given double precision value \a a. It returns 1.0
// if \a a is greater than zero, 0.0 if \a a is zero, -1.0 if \a a is less than zero, and \c NaN
// if a is \c NaN.
*/
constexpr double sign(double a) noexcept
{
	if (0.0 < a)
		return 1.0;
	else if (a < 0.0)
		return -1.0;
	else
		return a;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Evaluating the sign of the given extended precision value.
// \ingroup math_shims
//
// \param a The given extended precision value.
// \return 1 if the value is greater than zero, 0 if it is zero, and -1 if it is less than zero.
//
// The sign function evaluates the sign of the given extended precision value \a a. It returns 1.0
// if \a a is greater than zero, 0.0 if \a a is zero, -1.0 if \a a is less than zero, and \c NaN
// if a is \c NaN.
*/
constexpr long double sign(long double a) noexcept
{
	if (0.0L < a)
		return 1.0L;
	else if (a < 0.0L)
		return -1.0L;
	else
		return a;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
