// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISCOMPLEX_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISCOMPLEX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for complex types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is a complex data type.
// In case the type is a complex data type (ignoring the cv-qualifiers), the \a value member
// constant is set to \a true, the nested type definition \a Type is \a TrueType, and the class
// derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType,
// and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsComplex< complex<double> >::value      // Evaluates to 'true'
   mtrc::numeric::IsComplex< const complex<float> >::Type  // Results in TrueType
   mtrc::numeric::IsComplex< volatile complex<int> >       // Is derived from TrueType
   mtrc::numeric::IsComplex< float >::value                // Evaluates to 'false'
   mtrc::numeric::IsComplex< const double >::Type          // Results in FalseType
   mtrc::numeric::IsComplex< const volatile int >          // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsComplex : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsComplex type trait for the plain 'complex' type.
template <typename T> struct IsComplex<complex<T>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsComplex type trait for 'const complex'.
template <typename T> struct IsComplex<const complex<T>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsComplex type trait for 'volatile complex'.
template <typename T> struct IsComplex<volatile complex<T>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsComplex type trait for 'const volatile complex'
template <typename T> struct IsComplex<const volatile complex<T>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsComplex type trait.
// \ingroup type_traits
//
// The IsComplex_v variable template provides a convenient shortcut to access the nested
// \a value of the IsComplex class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsComplex<T>::value;
   constexpr bool value2 = mtrc::numeric::IsComplex_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsComplex_v = IsComplex<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
