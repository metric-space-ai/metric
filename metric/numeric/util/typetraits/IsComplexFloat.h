// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISCOMPLEXFLOAT_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISCOMPLEXFLOAT_H
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
// This type trait tests whether or not the given template parameter is of type \c complex<float>.
// In case the type is \c complex<float> (ignoring the cv-qualifiers), the \a value member
// constant is set to \a true, the nested type definition \a Type is \a TrueType, and the class
// derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and
// the class derives from \a FalseType.

   \code
   mtrc::numeric::IsComplexFloat< complex<float> >::value        // Evaluates to 'true'
   mtrc::numeric::IsComplexFloat< const complex<float> >::Type   // Results in TrueType
   mtrc::numeric::IsComplexFloat< volatile complex<float> >      // Is derived from TrueType
   mtrc::numeric::IsComplexFloat< float >::value                 // Evaluates to 'false'
   mtrc::numeric::IsComplexFloat< const complex<double> >::Type  // Results in FalseType
   mtrc::numeric::IsComplexFloat< const volatile complex<int> >  // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsComplexFloat : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsComplexFloat type trait for the plain 'complex<float>' type.
template <> struct IsComplexFloat<complex<float>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsComplexFloat type trait for 'const complex<float>'.
template <> struct IsComplexFloat<const complex<float>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsComplexFloat type trait for 'volatile complex<float>'.
template <> struct IsComplexFloat<volatile complex<float>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsComplexFloat type trait for 'const volatile complex<float>'
template <> struct IsComplexFloat<const volatile complex<float>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsComplexFloat type trait.
// \ingroup type_traits
//
// The IsComplexFloat_v variable template provides a convenient shortcut to access the nested
// \a value of the IsComplexFloat class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsComplexFloat<T>::value;
   constexpr bool value2 = mtrc::numeric::IsComplexFloat_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsComplexFloat_v = IsComplexFloat<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
