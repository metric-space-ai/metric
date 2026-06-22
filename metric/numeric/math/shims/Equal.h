// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_EQUAL_H
#define METRIC_NUMERIC_MATH_SHIMS_EQUAL_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>
#include <metric/numeric/math/Accuracy.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/typetraits/IsScalar.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/algorithms/Max.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  EQUAL SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Generic equality comparison.
// \ingroup math_shims
//
// \param a First value/object.
// \param b Second value/object.
// \return \a true if the two values/objects are equal, \a false if not.
//
// The equal shim represents an abstract interface for testing two values/objects for equality.
// Based on the setting of the relaxation flag \a RF, the function either performs a comparison
// via the equality operator (\a mtrc::numeric::strict) or or a special comparison is selected that takes
// the limited machine accuracy into account (\a mtrc::numeric::relaxed). In case the two values/objects
// are equal, the function returns \a true, otherwise it returns \a false.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename T1 // Type of the left-hand side value/object
		  ,
		  typename T2 // Type of the right-hand side value/object
		  ,
		  typename = EnableIf_t<IsScalar_v<T1> && IsScalar_v<T2>>>
constexpr bool equal(const T1 &a, const T2 &b)
{
	return a == b;
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check for two single precision floating point values.
// \ingroup math_shims
//
// \param a The left-hand side single precision floating point value.
// \param b The right-hand side single precision floating point value.
// \return \a true if the two values are equal, \a false if not.
//
// Equal function for the comparison of two single precision floating point numbers. Due to the
// limited machine accuracy, a direct comparison of two floating point numbers should be avoided.
// This function offers the possibility to compare two floating-point values with a certain
// accuracy margin.
//
// For more information on comparing float point numbers, see
//
//       http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
*/
template <RelaxationFlag RF> // Relaxation flag
inline bool equal(float a, float b)
{
	if (RF == relaxed) {
		const float acc(static_cast<float>(accuracy));
		return (std::fabs(a - b) <= max(acc, acc * std::fabs(a)));
	} else {
		return a == b;
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check for a single precision and a double precision floating point value.
// \ingroup math_shims
//
// \param a The left-hand side single precision floating point value.
// \param b The right-hand side double precision floating point value.
// \return \a true if the two values are equal, \a false if not.
//
// Equal function for the comparison of a single precision and a double precision floating point
// number. Due to the limited machine accuracy, a direct comparison of two floating point numbers
// should be avoided. This function offers the possibility to compare two floating-point values
// with a certain accuracy margin.
//
// For more information on comparing float point numbers, see
//
//       http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
*/
template <RelaxationFlag RF> // Relaxation flag
inline bool equal(float a, double b)
{
	return equal<RF>(a, static_cast<float>(b));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check for a single precision and an extended precision floating point value.
// \ingroup math_shims
//
// \param a The left-hand side single precision floating point value.
// \param b The right-hand side extended precision floating point value.
// \return \a true if the two values are equal, \a false if not.
//
// Equal function for the comparison of a single precision and an extended precision floating point
// number. Due to the limited machine accuracy, a direct comparison of two floating point numbers
// should be avoided. This function offers the possibility to compare two floating-point values
// with a certain accuracy margin.
//
// For more information on comparing float point numbers, see
//
//       http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
*/
template <RelaxationFlag RF> // Relaxation flag
inline bool equal(float a, long double b)
{
	return equal<RF>(a, static_cast<float>(b));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check for a double precision and a single precision floating point value.
// \ingroup math_shims
//
// \param a The left-hand side double precision floating point value.
// \param b The right-hand side single precision floating point value.
// \return \a true if the two values are equal, \a false if not.
//
// Equal function for the comparison of a double precision and a single precision floating point
// number. Due to the limited machine accuracy, a direct comparison of two floating point numbers
// should be avoided. This function offers the possibility to compare two floating-point values
// with a certain accuracy margin.
*/
template <RelaxationFlag RF> // Relaxation flag
inline bool equal(double a, float b)
{
	return equal<RF>(static_cast<float>(a), b);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check for two double precision floating point values.
// \ingroup math_shims
//
// \param a The left-hand side double precision floating point value.
// \param b The right-hand side double precision floating point value.
// \return \a true if the two values are equal, \a false if not.
//
// Equal function for the comparison of two double precision floating point numbers. Due to the
// limited machine accuracy, a direct comparison of two floating point numbers should be avoided.
// This function offers the possibility to compare two floating-point values with a certain
// accuracy margin.
//
// For more information on comparing float point numbers, see
//
//       http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
*/
template <RelaxationFlag RF> // Relaxation flag
inline bool equal(double a, double b)
{
	if (RF == relaxed) {
		const double acc(static_cast<double>(accuracy));
		return (std::fabs(a - b) <= max(acc, acc * std::fabs(a)));
	} else {
		return a == b;
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check for a double precision and an extended precision floating point value.
// \ingroup math_shims
//
// \param a The left-hand side double precision floating point value.
// \param b The right-hand side extended precision floating point value.
// \return \a true if the two values are equal, \a false if not.
//
// Equal function for the comparison of a double precision and an extended precision floating point
// number. Due to the limited machine accuracy, a direct comparison of two floating point numbers
// should be avoided. This function offers the possibility to compare two floating-point values
// with a certain accuracy margin.
//
// For more information on comparing float point numbers, see
//
//       http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
*/
template <RelaxationFlag RF> // Relaxation flag
inline bool equal(double a, long double b)
{
	return equal<RF>(a, static_cast<double>(b));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check for an extended precision and a single precision floating point value.
// \ingroup math_shims
//
// \param a The left-hand side extended precision floating point value.
// \param b The right-hand side single precision floating point value.
// \return \a true if the two values are equal, \a false if not.
//
// Equal function for the comparison of an extended precision and a single precision floating point
// number. Due to the limited machine accuracy, a direct comparison of two floating point numbers
// should be avoided. This function offers the possibility to compare two floating-point values
// with a certain accuracy margin.
//
// For more information on comparing float point numbers, see
//
//       http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
*/
template <RelaxationFlag RF> // Relaxation flag
inline bool equal(long double a, float b)
{
	return equal<RF>(static_cast<float>(a), b);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check for an extended precision and a double precision floating point value.
// \ingroup math_shims
//
// \param a The left-hand side extended precision floating point value.
// \param b The right-hand side double precision floating point value.
// \return \a true if the two values are equal, \a false if not.
//
// Equal function for the comparison of an extended precision and a double precision floating point
// number. Due to the limited machine accuracy, a direct comparison of two floating point numbers
// should be avoided. This function offers the possibility to compare two floating-point values
// with a certain accuracy margin.
//
// For more information on comparing float point numbers, see
//
//       http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
*/
template <RelaxationFlag RF> // Relaxation flag
inline bool equal(long double a, double b)
{
	return equal<RF>(static_cast<double>(a), b);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check for two long double precision floating point values.
// \ingroup math_shims
//
// \param a The left-hand side extended precision floating point value.
// \param b The right-hand side extended precision floating point value.
// \return \a true if the two values are equal, \a false if not.
//
// Equal function for the comparison of two long double precision floating point numbers. Due
// to the limited machine accuracy, a direct comparison of two floating point numbers should be
// avoided. This function offers the possibility to compare two floating-point values with a
// certain accuracy margin.
//
// For more information on comparing float point numbers, see
//
//       http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
*/
template <RelaxationFlag RF> // Relaxation flag
inline bool equal(long double a, long double b)
{
	if (RF == relaxed) {
		const long double acc(static_cast<long double>(accuracy));
		return (std::fabs(a - b) <= max(acc, acc * std::fabs(a)));
	} else {
		return a == b;
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check for a complex and a scalar value.
// \ingroup math_shims
//
// \param a The left-hand side complex value.
// \param b The right-hand side scalar value.
// \return \a true if the two values are equal, \a false if not.
//
// Equal function for the comparison of a complex and a scalar value. The function compares the
// real part of the complex value with the scalar. In case these two values match and in case
// the imaginary part is zero, the function returns \a true. Otherwise it returns \a false.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename T1 // Type of the left-hand side complex value
		  ,
		  typename T2> // Type of the right-hand side scalar value
inline bool equal(complex<T1> a, T2 b)
{
	return equal<RF>(real(a), b) && equal<RF>(imag(a), T1());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check for a scalar and a complex value.
// \ingroup math_shims
//
// \param a The left-hand side scalar value.
// \param b The right-hand side complex value.
// \return \a true if the two values are equal, \a false if not.
//
// Equal function for the comparison of a scalar and a complex value. The function compares the
// scalar with the real part of the complex value. In case these two values match and in case
// the imaginary part is zero, the function returns \a true. Otherwise it returns \a false.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename T1 // Type of the left-hand side scalar value
		  ,
		  typename T2> // Type of the right-hand side complex value
inline bool equal(T1 a, complex<T2> b)
{
	return equal<RF>(a, real(b)) && equal<RF>(imag(b), T2());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check for two complex values.
// \ingroup math_shims
//
// \param a The left-hand side complex value.
// \param b The right-hand side complex value.
// \return \a true if the two values are equal, \a false if not.
//
// Equal function for the comparison of two complex numbers. Due to the limited machine accuracy,
// a direct comparison of two floating point numbers should be avoided. This function offers the
// possibility to compare two floating-point values with a certain accuracy margin.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename T1 // Type of the left-hand side complex value
		  ,
		  typename T2> // Type of the right-hand side complex value
inline bool equal(complex<T1> a, complex<T2> b)
{
	return equal<RF>(real(a), real(b)) && equal<RF>(imag(a), imag(b));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Generic equality check.
// \ingroup math_shims
//
// \param a First value/object.
// \param b Second value/object.
// \return \a true if the two values/objects are equal, \a false if not.
//
// The equal shim represents an abstract interface for testing two values/objects for equality.
// In case the two values/objects are equal, the function returns \a true, otherwise it returns
// \a false. Per default, the comparison of the two values/objects uses the equality operator
// operator==(). For built-in floating point data types a special comparison is selected that
// takes the limited machine accuracy into account.
*/
template <typename T1 // Type of the left-hand side value/object
		  ,
		  typename T2> // Type of the right-hand side value/object
constexpr bool equal(const T1 &a, const T2 &b)
{
	return equal<relaxed>(a, b);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
