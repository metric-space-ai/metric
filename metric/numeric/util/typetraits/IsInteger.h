// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISINTEGER_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISINTEGER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for integer types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is an integer type (i.e.,
// either (signed) int or unsigned int, possibly cv-qualified). In case the type is an integer
// type (ignoring the cv-qualifiers), the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsInteger<int>::value                 // Evaluates to 'true'
   mtrc::numeric::IsInteger<const unsigned int>::Type   // Results in TrueType
   mtrc::numeric::IsInteger<const volatile signed int>  // Is derived from TrueType
   mtrc::numeric::IsInteger<unsigned short>::value      // Evaluates to 'false'
   mtrc::numeric::IsInteger<const long>::Type           // Results in FalseType
   mtrc::numeric::IsInteger<volatile float>             // Is derived from FalseType
   \endcode

// Note the difference between the IsInteger and IsIntegral type traits: Whereas the IsInteger
// type trait specifically tests whether the given data type is either int or unsigned int
// (possibly cv-qualified), the IsIntegral type trait tests whether the given template argument
// is an integral data type (char, short, int, long, etc.).
*/
template <typename T> struct IsInteger : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsInteger type trait for the plain 'int' type.
template <> struct IsInteger<int> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsInteger type trait for 'const int'.
template <> struct IsInteger<const int> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsInteger type trait for 'volatile int'.
template <> struct IsInteger<volatile int> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsInteger type trait for 'const volatile int'.
template <> struct IsInteger<const volatile int> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsInteger type trait for the plain 'unsigned int' type.
template <> struct IsInteger<unsigned int> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsInteger type trait for 'const unsigned int'.
template <> struct IsInteger<const unsigned int> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsInteger type trait for 'volatile unsigned int'.
template <> struct IsInteger<volatile unsigned int> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsInteger type trait for 'const volatile unsigned int'.
template <> struct IsInteger<const volatile unsigned int> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsInteger type trait.
// \ingroup type_traits
//
// The IsInteger_v variable template provides a convenient shortcut to access the nested
// \a value of the IsInteger class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsInteger<T>::value;
   constexpr bool value2 = mtrc::numeric::IsInteger_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsInteger_v = IsInteger<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
