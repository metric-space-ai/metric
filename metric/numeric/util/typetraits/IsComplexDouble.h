// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISCOMPLEXDOUBLE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISCOMPLEXDOUBLE_H
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
/*!\brief Compile time check for single precision complex types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is of type \c complex<double>.
// In case the type is \c complex<double> (ignoring the cv-qualifiers), the \a value member
// constant is set to \a true, the nested type definition \a Type is \a TrueType, and the class
// derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and
// the class derives from \a FalseType.

   \code
   mtrc::numeric::IsComplexDouble< complex<double> >::value       // Evaluates to 'true'
   mtrc::numeric::IsComplexDouble< const complex<double> >::Type  // Results in TrueType
   mtrc::numeric::IsComplexDouble< volatile complex<double> >     // Is derived from TrueType
   mtrc::numeric::IsComplexDouble< double >::value                // Evaluates to 'false'
   mtrc::numeric::IsComplexDouble< const complex<float> >::Type   // Results in FalseType
   mtrc::numeric::IsComplexDouble< const volatile complex<int> >  // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsComplexDouble : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsComplexDouble type trait for the plain 'complex<double>' type.
template <> struct IsComplexDouble<complex<double>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsComplexDouble type trait for 'const complex<double>'.
template <> struct IsComplexDouble<const complex<double>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsComplexDouble type trait for 'volatile complex<double>'.
template <> struct IsComplexDouble<volatile complex<double>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsComplexDouble type trait for 'const volatile complex<double>'
template <> struct IsComplexDouble<const volatile complex<double>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsComplexDouble type trait.
// \ingroup type_traits
//
// The IsComplexDouble_v variable template provides a convenient shortcut to access the nested
// \a value of the IsComplexDouble class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsComplexDouble<T>::value;
   constexpr bool value2 = mtrc::numeric::IsComplexDouble_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsComplexDouble_v = IsComplexDouble<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
