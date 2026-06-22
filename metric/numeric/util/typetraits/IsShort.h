// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISSHORT_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISSHORT_H
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
/*!\brief Compile time check for short integer types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is a short integer type
// (i.e., either (signed) short or unsigned short, possibly cv-qualified). In case the type
// is a short integer type (ignoring the cv-qualifiers), the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.

   \code
   mtrc::numeric::IsShort<short>::value                 // Evaluates to 'true'
   mtrc::numeric::IsShort<const unsigned short>::Type   // Results in TrueType
   mtrc::numeric::IsShort<const volatile signed short>  // Is derived from TrueType
   mtrc::numeric::IsShort<unsigned int>::value          // Evaluates to 'false'
   mtrc::numeric::IsShort<const long>::Type             // Results in FalseType
   mtrc::numeric::IsShort<volatile float>               // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsShort : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsShort type trait for the plain 'short' type.
template <> struct IsShort<short> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsShort type trait for 'const short'.
template <> struct IsShort<const short> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsShort type trait for 'volatile short'.
template <> struct IsShort<volatile short> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsShort type trait for 'const volatile short'.
template <> struct IsShort<const volatile short> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsShort type trait for the plain 'unsigned short' type.
template <> struct IsShort<unsigned short> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsShort type trait for 'const unsigned short'.
template <> struct IsShort<const unsigned short> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsShort type trait for 'volatile unsigned short'.
template <> struct IsShort<volatile unsigned short> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsShort type trait for 'const volatile unsigned short'.
template <> struct IsShort<const volatile unsigned short> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsShort type trait.
// \ingroup type_traits
//
// The IsShort_v variable template provides a convenient shortcut to access the nested \a value
// of the IsShort class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsShort<T>::value;
   constexpr bool value2 = mtrc::numeric::IsShort_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsShort_v = IsShort<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
