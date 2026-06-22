// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISDOUBLE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISDOUBLE_H
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
/*!\brief Compile time check for double precision floating point types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is of double type. In
// case the type is double (ignoring the cv-qualifiers), the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.

   \code
   mtrc::numeric::IsDouble<double>::value          // Evaluates to 'true'
   mtrc::numeric::IsDouble<const double>::Type     // Results in TrueType
   mtrc::numeric::IsDouble<const volatile double>  // Is derived from TrueType
   mtrc::numeric::IsDouble<float>::value           // Evaluates to 'false'
   mtrc::numeric::IsDouble<const int>::Type        // Results in FalseType
   mtrc::numeric::IsDouble<volatile short>         // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsDouble : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsDouble type trait for the plain 'double' type.
template <> struct IsDouble<double> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsDouble type trait for 'const double'.
template <> struct IsDouble<const double> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsDouble type trait for 'volatile double'.
template <> struct IsDouble<volatile double> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsDouble type trait for 'const volatile double'.
template <> struct IsDouble<const volatile double> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsDouble type trait.
// \ingroup type_traits
//
// The IsDouble_v variable template provides a convenient shortcut to access the nested \a value
// of the IsDouble class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsDouble<T>::value;
   constexpr bool value2 = mtrc::numeric::IsDouble_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsDouble_v = IsDouble<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
