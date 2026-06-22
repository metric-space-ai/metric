// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISBOOLEAN_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISBOOLEAN_H
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
/*!\brief Compile time check for boolean types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is of boolean type. In
// case the type is a boolean (ignoring the cv-qualifiers), the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives
// from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the
// class derives from \a FalseType.

   \code
   mtrc::numeric::IsBoolean<bool>::value          // Evaluates to 'true'
   mtrc::numeric::IsBoolean<const bool>::Type     // Results in TrueType
   mtrc::numeric::IsBoolean<const volatile bool>  // Is derived from TrueType
   mtrc::numeric::IsBoolean<float>::value         // Evaluates to 'false' (float is not a boolean)
   mtrc::numeric::IsBoolean<const int>::Type      // Results in FalseType
   mtrc::numeric::IsBoolean<volatile short>       // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsBoolean : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsBoolean type trait for the plain 'bool' type.
template <> struct IsBoolean<bool> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsBoolean type trait for 'const bool'.
template <> struct IsBoolean<const bool> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsBoolean type trait for 'volatile bool'.
template <> struct IsBoolean<volatile bool> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsBoolean type trait for 'const volatile bool'
template <> struct IsBoolean<const volatile bool> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsBoolean type trait.
// \ingroup type_traits
//
// The IsBoolean_v variable template provides a convenient shortcut to access the nested
// \a value of the IsBoolean class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsBoolean<T>::value;
   constexpr bool value2 = mtrc::numeric::IsBoolean_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsBoolean_v = IsBoolean<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
