// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISLONGDOUBLE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISLONGDOUBLE_H
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
/*!\brief Compile time check for extended precision floating point types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is of long double type. In
// case the type is long double (ignoring the cv-qualifiers), the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives
// from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the
// class derives from \a FalseType.

   \code
   mtrc::numeric::IsLongDouble<long double>::value          // Evaluates to 'true'
   mtrc::numeric::IsLongDouble<const long double>::Type     // Results in TrueType
   mtrc::numeric::IsLongDouble<const volatile long double>  // Is derived from TrueType
   mtrc::numeric::IsLongDouble<float>::value                // Evaluates to 'false'
   mtrc::numeric::IsLongDouble<const unsigned int>::Type    // Results in FalseType
   mtrc::numeric::IsLongDouble<const volatile short>        // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsLongDouble : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsLongDouble type trait for the plain 'long double' type.
template <> struct IsLongDouble<long double> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsLongDouble type trait for 'const long double'.
template <> struct IsLongDouble<const long double> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsLongDouble type trait for 'volatile long double'.
template <> struct IsLongDouble<volatile long double> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsLongDouble type trait for 'const volatile long double'.
template <> struct IsLongDouble<const volatile long double> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsLongDouble type trait.
// \ingroup type_traits
//
// The IsLongDouble_v variable template provides a convenient shortcut to access the nested
// \a value of the IsLongDouble class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsLongDouble<T>::value;
   constexpr bool value2 = mtrc::numeric::IsLongDouble_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsLongDouble_v = IsLongDouble<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
