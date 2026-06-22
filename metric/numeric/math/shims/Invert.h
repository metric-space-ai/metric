// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_INVERT_H
#define METRIC_NUMERIC_MATH_SHIMS_INVERT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/shims/Pow2.h>
#include <metric/numeric/math/typetraits/IsScalar.h>
#include <metric/numeric/math/typetraits/UnderlyingBuiltin.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/IsBuiltin.h>
#include <metric/numeric/util/typetraits/IsIntegral.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  INV SHIMS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Inverting the given value/object.
// \ingroup math_shims
//
// \param a The value/object to be inverted.
// \return The inverse of the given value/object.
//
// The \a inv shim represents an abstract interface for inverting a value/object of any given
// data type. For integral values this results in \f$ \frac{1}{a} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
template <typename T, EnableIf_t<IsScalar_v<T>> * = nullptr>
METRIC_NUMERIC_ALWAYS_INLINE constexpr decltype(auto) inv(T a) noexcept(IsBuiltin_v<T>)
{
	METRIC_NUMERIC_USER_ASSERT(a != T{}, "Division by zero detected");

	using BT = UnderlyingBuiltin_t<T>;
	using ST = If_t<IsIntegral_v<BT>, double, BT>;

	return (ST(1) / a);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Inverting the given single precision complex number.
// \ingroup math_shims
//
// \param a The single precision complex number to be inverted.
// \return The inverse of the given value.
//
// The \a inv shim represents an abstract interface for inverting a value/object of any given
// data type. For a single precision floating point complex number \f$ z = x + yi \f$ this
// results in \f$ \frac{\overline{z}}{x^2+y^2} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
METRIC_NUMERIC_ALWAYS_INLINE constexpr complex<float> inv(const complex<float> &a) noexcept
{
	const float abs(pow2(real(a)) + pow2(imag(a)));
	METRIC_NUMERIC_USER_ASSERT(abs != 0.0F, "Division by zero detected");

	const float iabs(1.0F / abs);
	return complex<float>(iabs * real(a), -iabs * imag(a));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Inverting the given double precision complex number.
// \ingroup math_shims
//
// \param a The double precision complex number to be inverted.
// \return The inverse of the given value.
//
// The \a inv shim represents an abstract interface for inverting a value/object of any given
// data type. For a double precision floating point complex number \f$ z = x + yi \f$ this
// results in \f$ \frac{\overline{z}}{x^2+y^2} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
METRIC_NUMERIC_ALWAYS_INLINE constexpr complex<double> inv(const complex<double> &a) noexcept
{
	const double abs(pow2(real(a)) + pow2(imag(a)));
	METRIC_NUMERIC_USER_ASSERT(abs != 0.0, "Division by zero detected");

	const double iabs(1.0 / abs);
	return complex<double>(iabs * real(a), -iabs * imag(a));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Inverting the given extended precision complex number.
// \ingroup math_shims
//
// \param a The extended precision complex number to be inverted.
// \return The inverse of the given value.
//
// The \a inv shim represents an abstract interface for inverting a value/object of any given
// data type. For an extended precision floating point complex number \f$ z = x + yi \f$ this
// results in \f$ \frac{\overline{z}}{x^2+y^2} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
METRIC_NUMERIC_ALWAYS_INLINE constexpr complex<long double> inv(const complex<long double> &a) noexcept
{
	const long double abs(pow2(real(a)) + pow2(imag(a)));
	METRIC_NUMERIC_USER_ASSERT(abs != 0.0L, "Division by zero detected");

	const long double iabs(1.0L / abs);
	return complex<long double>(iabs * real(a), -iabs * imag(a));
}
//*************************************************************************************************

//=================================================================================================
//
//  INVERT SHIMS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief In-place inversion of the given single precision value.
// \ingroup math_shims
//
// \param a The single precision value to be inverted.
// \return The inverse of the given value.
//
// The \a invert shim represents an abstract interface for inverting a value/object of any
// given data type in-place. For single precision floating point values this results in
// \f$ \frac{1}{a} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
METRIC_NUMERIC_ALWAYS_INLINE void invert(float &a) noexcept { a = inv(a); }
//*************************************************************************************************

//*************************************************************************************************
/*!\brief In-place inversion of the given double precision value.
// \ingroup math_shims
//
// \param a The double precision value to be inverted.
// \return The inverse of the given value.
//
// The \a invert shim represents an abstract interface for inverting a value/object of any
// given data type in-place. For double precision floating point values this results in
// \f$ \frac{1}{a} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
METRIC_NUMERIC_ALWAYS_INLINE void invert(double &a) noexcept { a = inv(a); }
//*************************************************************************************************

//*************************************************************************************************
/*!\brief In-place inversion of the given extended precision value.
// \ingroup math_shims
//
// \param a The extended precision value to be inverted.
// \return The inverse of the given value.
//
// The \a invert shim represents an abstract interface for inverting a value/object of any
// given data type in-place. For extended precision floating point values this results in
// \f$ \frac{1}{a} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
METRIC_NUMERIC_ALWAYS_INLINE void invert(long double &a) noexcept { a = inv(a); }
//*************************************************************************************************

//*************************************************************************************************
/*!\brief In-place inversion of the given single precision complex number.
// \ingroup math_shims
//
// \param a The single precision complex number to be inverted.
// \return The inverse of the given value.
//
// The \a invert shim represents an abstract interface for inverting a value/object of any given
// data type in-place. For a single precision floating point complex number \f$ z = x + yi \f$
// this results in \f$ \frac{\overline{z}}{x^2+y^2} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
METRIC_NUMERIC_ALWAYS_INLINE void invert(complex<float> &a) noexcept { a = inv(a); }
//*************************************************************************************************

//*************************************************************************************************
/*!\brief In-place inversion of the given double precision complex number.
// \ingroup math_shims
//
// \param a The double precision complex number to be inverted.
// \return The inverse of the given value.
//
// The \a invert shim represents an abstract interface for inverting a value/object of any given
// data type in-place. For a double precision floating point complex number \f$ z = x + yi \f$
// this results in \f$ \frac{\overline{z}}{x^2+y^2} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
METRIC_NUMERIC_ALWAYS_INLINE void invert(complex<double> &a) noexcept { a = inv(a); }
//*************************************************************************************************

//*************************************************************************************************
/*!\brief In-place inversion of the given extended precision complex number.
// \ingroup math_shims
//
// \param a The extended precision complex number to be inverted.
// \return The inverse of the given value.
//
// The \a invert shim represents an abstract interface for inverting a value/object of any given
// data type in-place. For an extended precision floating point complex number \f$ z = x + yi \f$
// this results in \f$ \frac{\overline{z}}{x^2+y^2} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
METRIC_NUMERIC_ALWAYS_INLINE void invert(complex<long double> &a) noexcept { a = inv(a); }
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
