// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_LESS_H
#define METRIC_NUMERIC_MATH_SHIMS_LESS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/typetraits/CommonType.h>
#include <metric/numeric/util/typetraits/IsBuiltin.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  LESS SHIM
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default less-than comparison for any data type.
// \ingroup math_shims
//
// \param a First value.
// \param b Second value.
// \return \a true if the first value is less than the second, \a false if not.
//
// Default implementation of a less-than comparison of two data values.
*/
template <typename T>
METRIC_NUMERIC_ALWAYS_INLINE constexpr bool less_backend(const T &a, const T &b) noexcept(IsBuiltin_v<T>)
{
	return a < b;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Less-than comparison for single precision floating point values.
// \ingroup math_shims
//
// \param a First value.
// \param b Second value.
// \return \a true if the first value is less than the second, \a false if not.
//
// Less-than function for the comparison of two single precision floating point numbers. Due
// to the limited machine accuracy, a direct comparison of two floating point numbers should
// be avoided. This functions offers the possibility to compare two floating-point values with
// a certain accuracy margin.
*/
METRIC_NUMERIC_ALWAYS_INLINE constexpr bool less_backend(float a, float b) noexcept { return (b - a) > 1E-8F; }
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Less-than comparison for double precision floating point values.
// \ingroup math_shims
//
// \param a First value.
// \param b Second value.
// \return \a true if the first value is less than the second, \a false if not.
//
// Less-than function for the comparison of two double precision floating point numbers. Due
// to the limited machine accuracy, a direct comparison of two floating point numbers should
// be avoided. This functions offers the possibility to compare two floating-point values with
// a certain accuracy margin.
*/
METRIC_NUMERIC_ALWAYS_INLINE constexpr bool less_backend(double a, double b) noexcept { return (b - a) > 1E-8; }
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Less-than comparison for long double precision floating point values.
// \ingroup math_shims
//
// \param a First value.
// \param b Second value.
// \return \a true if the first value is less than the second, \a false if not.
//
// Less-than function for the comparison of two long double precision floating point numbers. Due
// to the limited machine accuracy, a direct comparison of two floating point numbers should be
// avoided. This functions offers the possibility to compare two floating-point values with a
// certain accuracy margin.
*/
METRIC_NUMERIC_ALWAYS_INLINE constexpr bool less_backend(long double a, long double b) noexcept
{
	return (b - a) > 1E-10;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Generic less-than comparison.
// \ingroup math_shims
//
// \param a First value.
// \param b Second value.
// \return \a true if the first value is less than the second, \a false if not.
//
// Generic less-than comparison between to numeric values. Depending on the types of the
// two arguments, a special comparison for floating point values is selected that takes
// the limited machine accuracy into account.
*/
template <typename T1, typename T2>
METRIC_NUMERIC_ALWAYS_INLINE constexpr bool less(const T1 &a, const T2 &b) noexcept(IsBuiltin_v<CommonType_t<T1, T2>>)
{
	return less_backend<CommonType_t<T1, T2>>(a, b);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
