// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISLONG_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISLONG_H
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
/*!\brief Compile time check for long integer types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is a long integer type
// (i.e., either (signed) long or unsigned long, possibly cv-qualified). In case the type
// is a long integer type (ignoring the cv-qualifiers), the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.

   \code
   mtrc::numeric::IsLong<long>::value                 // Evaluates to 'true'
   mtrc::numeric::IsLong<const unsigned long>::Type   // Results in TrueType
   mtrc::numeric::IsLong<const volatile signed long>  // Is derived from TrueType
   mtrc::numeric::IsLong<unsigned short>::value       // Evaluates to 'false'
   mtrc::numeric::IsLong<const int>::Type             // Results in FalseType
   mtrc::numeric::IsLong<volatile float>              // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsLong : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsLong type trait for the plain 'long' type.
template <> struct IsLong<long> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsLong type trait for 'const long'.
template <> struct IsLong<const long> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsLong type trait for 'volatile long'.
template <> struct IsLong<volatile long> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsLong type trait for 'const volatile long'.
template <> struct IsLong<const volatile long> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsLong type trait for the plain 'unsigned long' type.
template <> struct IsLong<unsigned long> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsLong type trait for 'const unsigned long'.
template <> struct IsLong<const unsigned long> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsLong type trait for 'volatile unsigned long'.
template <> struct IsLong<volatile unsigned long> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsLong type trait for 'const volatile unsigned long'.
template <> struct IsLong<const volatile unsigned long> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsLong type trait.
// \ingroup type_traits
//
// The IsLong_v variable template provides a convenient shortcut to access the nested \a value
// of the IsLong class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsLong<T>::value;
   constexpr bool value2 = mtrc::numeric::IsLong_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsLong_v = IsLong<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
