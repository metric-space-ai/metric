// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISPADDED_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISPADDED_H
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
/*!\brief Compile time check for data types with padding.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type employs or simulates padding such that no
// special treatment of remainder elements is necessary for vectorized operations on the type.
// In case the data type is padded, the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
// Examples:

   \code
   mtrc::numeric::IsPadded< mtrc::numeric::StaticVector<int,3UL> >::value     // Evaluates to 1
   mtrc::numeric::IsPadded< const mtrc::numeric::HybridVector<float> >::Type  // Results in TrueType
   mtrc::numeric::IsPadded< volatile mtrc::numeric::DynamicMatrix<double> >   // Is derived from TrueType
   mtrc::numeric::IsPadded< int >::value                              // Evaluates to 0
   mtrc::numeric::IsPadded< const CompressedVector<int> >::Type       // Results in FalseType
   mtrc::numeric::IsPadded< volatile CompressedMatrix<double> >       // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsPadded : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsPadded type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsPadded<const T> : public IsPadded<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsPadded type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsPadded<volatile T> : public IsPadded<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsPadded type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsPadded<const volatile T> : public IsPadded<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsPadded type trait.
// \ingroup math_type_traits
//
// The IsPadded_v variable template provides a convenient shortcut to access the nested \a value
// of the IsPadded class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsPadded<T>::value;
   constexpr bool value2 = mtrc::numeric::IsPadded_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsPadded_v = IsPadded<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
