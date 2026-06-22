// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_LIMITS_H
#define METRIC_NUMERIC_UTIL_LIMITS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <limits>
#include <metric/numeric/system/Platform.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Numerical limits of built-in data types.
// \ingroup util
//
// The Limits class provides numerical limits for the following built-in data types:
//
// <ul>
//    <li>Integral data types</li>
//    <ul>
//       <li>unsigned char, signed char, char, wchar_t</li>
//       <li>unsigned short, short</li>
//       <li>unsigned int, int</li>
//       <li>unsigned long, long</li>
//       <li>size_t, ptrdiff_t (for certain 64-bit compilers)</li>
//    </ul>
//    <li>Floating point data types</li>
//    <ul>
//       <li>float</li>
//       <li>double</li>
//       <li>long double</li>
//    </ul>
// </ul>
//
// Depending on the data type, the following limits can be used:
//
// - \b inf: The \a inf function is defined for all built-in data types. It returns the largest
//      possible positive value of the according data type.
// - \b ninf: The \a ninf function is defined for all signed integral and all floating point
//      data types. It returns the largest possible negative value of the according data type.
// - \b epsilon: The \a epsilon function is defined for all floating point data types and
//      returns the smallest possible difference between two values of the according data type.
// - \b accuracy: The \a accuracy function is defined for all floating point data types and
//      returns the computation accuracy of the corresponding data type. Due to the limited
//      floating point accuracy of a CPU this value is needed as computation threshold. This
//      value is used in most computations throughout the Metric numeric library.
// - \b fpuAccuracy: The \a fpuAccuracy function is defined for all floating point data types
//      and returns the floating point accuracy of the according point data type. Due to the
//      limited floating point accuracy of a CPU this value is needed as zero threshold in
//      computations.
//
// Code examples:

   \code
   // Positiv infinity value
   unsigned int ui = Limits<unsigned int>::inf();

   // Negative infinity value
   double d = Limits<double>::ninf();
   \endcode
*/
template <typename Type> struct Limits {};
//*************************************************************************************************

//=================================================================================================
//
//  SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<unsigned char> specialization.
// \ingroup util
*/
template <> struct Limits<unsigned char> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive unsigned char value. */
	static constexpr unsigned char inf() { return std::numeric_limits<unsigned char>::max(); }
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<char> specialization.
// \ingroup util
*/
template <> struct Limits<char> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive char value. */
	static constexpr char inf() { return std::numeric_limits<char>::max(); }

	/*!\brief Negative infinity value.
	// \return The largest possible negative char value. */
	static constexpr char ninf() { return std::numeric_limits<char>::min(); }
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<signed char> specialization.
// \ingroup util
*/
template <> struct Limits<signed char> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive signed char value. */
	static constexpr signed char inf() { return std::numeric_limits<signed char>::max(); }

	/*!\brief Negative infinity value.
	// \return The largest possible negative signed char value. */
	static constexpr signed char ninf() { return std::numeric_limits<signed char>::min(); }
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<wchar_t> specialization.
// \ingroup util
*/
template <> struct Limits<wchar_t> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive wchar_t value. */
	static constexpr wchar_t inf() { return std::numeric_limits<wchar_t>::max(); }

	/*!\brief Negative infinity value.
	// \return The largest possible negative wchar_t value. */
	static constexpr wchar_t ninf() { return std::numeric_limits<wchar_t>::min(); }
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<unsigned short> specialization.
// \ingroup util
*/
template <> struct Limits<unsigned short> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive unsigned short value. */
	static constexpr unsigned short inf() { return std::numeric_limits<unsigned short>::max(); }
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<short> specialization.
// \ingroup util
*/
template <> struct Limits<short> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive short value. */
	static constexpr short inf() { return std::numeric_limits<short>::max(); }

	/*!\brief Negative infinity value.
	// \return The largest possible negative short value. */
	static constexpr short ninf() { return std::numeric_limits<short>::min(); }
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<unsigned int> specialization.
// \ingroup util
*/
template <> struct Limits<unsigned int> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive unsigned int value. */
	static constexpr unsigned int inf() { return std::numeric_limits<unsigned int>::max(); }
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<int> specialization.
// \ingroup util
*/
template <> struct Limits<int> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive int value. */
	static constexpr int inf() { return std::numeric_limits<int>::max(); }

	/*!\brief Negative infinity value.
	// \return The largest possible negative int value. */
	static constexpr int ninf() { return std::numeric_limits<int>::min(); }
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<unsigned long> specialization.
// \ingroup util
*/
template <> struct Limits<unsigned long> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive unsigned long value. */
	static constexpr unsigned long inf() { return std::numeric_limits<unsigned long>::max(); }
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<long> specialization.
// \ingroup util
*/
template <> struct Limits<long> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive long value. */
	static constexpr long inf() { return std::numeric_limits<long>::max(); }

	/*!\brief Negative infinity value.
	// \return The largest possible negative long value. */
	static constexpr long ninf() { return std::numeric_limits<long>::min(); }
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_WIN32_PLATFORM || METRIC_NUMERIC_WIN64_PLATFORM
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<size_t> specialization.
// \ingroup util
*/
template <> struct Limits<size_t> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive size_t value. */
	static constexpr size_t inf() { return std::numeric_limits<size_t>::max(); }
};
/*! \endcond */
#endif
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_WIN32_PLATFORM || METRIC_NUMERIC_WIN64_PLATFORM
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<ptrdiff_t> specialization.
// \ingroup util
*/
template <> struct Limits<ptrdiff_t> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive ptrdiff_t value. */
	static constexpr ptrdiff_t inf() { return std::numeric_limits<ptrdiff_t>::max(); }

	/*!\brief Negative infinity value.
	// \return The largest possible negative ptrdiff_t value. */
	static constexpr ptrdiff_t ninf() { return std::numeric_limits<ptrdiff_t>::min(); }
};
/*! \endcond */
#endif
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<float> specialization.
// \ingroup util
*/
template <> struct Limits<float> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive float value. */
	static constexpr float inf() { return std::numeric_limits<float>::max(); }

	/*!\brief Negative infinity value.
	// \return The largest possible negative float value. */
	static constexpr float ninf() { return -std::numeric_limits<float>::max(); }

	/*!\brief Machine epsilon.
	// \return The smallest possible difference between two float values. */
	static constexpr float epsilon() { return std::numeric_limits<float>::epsilon(); }

	/*!\brief The compuation accuracy of the Metric numeric library.
	// \return The computation threshold for single precision floating point values. */
	static constexpr float accuracy() { return 1E-6F; }

	/*!\brief The machine floating point accuracy.
	// \return The machine accuracy for single precision floating point values. */
	static constexpr float fpuAccuracy() { return 1E-12F; }
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<double> specialization.
// \ingroup util
*/
template <> struct Limits<double> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive double value. */
	static constexpr double inf() { return std::numeric_limits<double>::max(); }

	/*!\brief Negative infinity value.
	// \return The largest possible negative double value. */
	static constexpr double ninf() { return -std::numeric_limits<double>::max(); }

	/*!\brief Machine epsilon.
	// \return The smallest possible difference between two double values. */
	static constexpr double epsilon() { return std::numeric_limits<double>::epsilon(); }

	/*!\brief The compuation accuracy of the Metric numeric library.
	// \return The computation threshold for double precision floating point values. */
	static constexpr double accuracy() { return 1E-8; }

	/*!\brief The machine floating point accuracy.
	// \return The machine accuracy for double precision floating point values. */
	static constexpr double fpuAccuracy() { return 1E-15; }
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Limits<long double> specialization.
// \ingroup util
*/
template <> struct Limits<long double> {
	/*!\brief Positive infinity value.
	// \return The largest possible positive long double value. */
	static constexpr long double inf() { return std::numeric_limits<long double>::max(); }

	/*!\brief Negative infinity value.
	// \return The largest possible negative long double value. */
	static constexpr long double ninf() { return -std::numeric_limits<long double>::max(); }

	/*!\brief Machine epsilon.
	// \return The smallest possible difference between two long double values. */
	static constexpr long double epsilon() { return std::numeric_limits<long double>::epsilon(); }

	/*!\brief The compuation accuracy of the Metric numeric library.
	// \return The computation threshold for long double floating point values. */
	static constexpr long double accuracy() { return 1E-10L; }

	/*!\brief The machine floating point accuracy.
	// \return The machine accuracy for long double floating point values. */
	static constexpr long double fpuAccuracy() { return 1E-15L; }
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
