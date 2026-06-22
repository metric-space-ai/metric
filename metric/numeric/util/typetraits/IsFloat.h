// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISFLOAT_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISFLOAT_H
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
/*!\brief Compile time check for single precision floating point types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is of float type. In
// case the type is float (ignoring the cv-qualifiers), the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.

   \code
   mtrc::numeric::IsFloat<float>::value          // Evaluates to 'true'
   mtrc::numeric::IsFloat<const float>::Type     // Results in TrueType
   mtrc::numeric::IsFloat<const volatile float>  // Is derived from TrueType
   mtrc::numeric::IsFloat<double>::value         // Evaluates to 'false'
   mtrc::numeric::IsFloat<const int>::Type       // Results in FalseType
   mtrc::numeric::IsFloat<volatile short>        // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsFloat : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsFloat type trait for the plain 'float' type.
template <> struct IsFloat<float> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsFloat type trait for 'const float'.
template <> struct IsFloat<const float> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsFloat type trait for 'volatile float'.
template <> struct IsFloat<volatile float> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsFloat type trait for 'const volatile float'.
template <> struct IsFloat<const volatile float> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsFloat type trait.
// \ingroup type_traits
//
// The IsFloat_v variable template provides a convenient shortcut to access the nested \a value
// of the IsFloat class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsFloat<T>::value;
   constexpr bool value2 = mtrc::numeric::IsFloat_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsFloat_v = IsFloat<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
