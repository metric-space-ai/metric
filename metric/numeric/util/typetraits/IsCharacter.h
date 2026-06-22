// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISCHARACTER_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISCHARACTER_H
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
/*!\brief Compile time check for character types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is a character type
// (i.e., either char, signed char, unsigned char, or wchar_t, possibly cv-qualified).
// In case the type is a character type (ignoring the cv-qualifiers), the \a value member
// constant is set to \a true, the nested type definition \a Type is \a TrueType, and
// the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsCharacter<char>::value                // Evaluates to 'true'
   mtrc::numeric::IsCharacter<const unsigned char>::Type  // Results in TrueType
   mtrc::numeric::IsCharacter<const volatile wchar_t>     // Is derived from TrueType
   mtrc::numeric::IsCharacter<unsigned short>::value      // Evaluates to 'false'
   mtrc::numeric::IsCharacter<const int>::Type            // Results in FalseType
   mtrc::numeric::IsCharacter<volatile long>              // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsCharacter : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsCharacter type trait for the plain 'char' type.
template <> struct IsCharacter<char> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsCharacter type trait for the plain 'signed char' type.
template <> struct IsCharacter<signed char> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsCharacter type trait for the plain 'unsigned char' type.
template <> struct IsCharacter<unsigned char> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsCharacter type trait for the plain 'wchar_t' type.
template <> struct IsCharacter<wchar_t> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Partial specialization of the IsCharacter type trait 'const' qualified types.
template <typename T> struct IsCharacter<const T> : public IsCharacter<T>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Partial specialization of the IsCharacter type trait 'volatile' qualified types.
template <typename T> struct IsCharacter<volatile T> : public IsCharacter<T>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Partial specialization of the IsCharacter type trait 'const volatile' types.
template <typename T> struct IsCharacter<const volatile T> : public IsCharacter<T>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsCharacter type trait.
// \ingroup type_traits
//
// The IsCharacter_v variable template provides a convenient shortcut to access the nested
// \a value of the IsCharacter class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsCharacter<T>::value;
   constexpr bool value2 = mtrc::numeric::IsCharacter_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsCharacter_v = IsCharacter<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
