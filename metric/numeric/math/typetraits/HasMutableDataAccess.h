// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASMUTABLEDATAACCESS_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASMUTABLEDATAACCESS_H
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
/*!\brief Compile time check for low-level access to mutable data.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type provides a low-level access to mutable data
// via a non-const 'data' member function. In case the according member function is provided,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType. Examples:

   \code
   mtrc::numeric::HasMutableDataAccess< StaticVector<float,3U> >::value      // Evaluates to 1
   mtrc::numeric::HasMutableDataAccess< const DynamicVector<double> >::Type  // Results in TrueType
   mtrc::numeric::HasMutableDataAccess< volatile DynamicMatrix<int> >        // Is derived from TrueType
   mtrc::numeric::HasMutableDataAccess< int >::value                         // Evaluates to 0
   mtrc::numeric::HasMutableDataAccess< const CompressedVector<int> >::Type  // Results in FalseType
   mtrc::numeric::HasMutableDataAccess< volatile CompressedMatrix<int> >     // Is derived from FalseType
   \endcode
*/
template <typename T> struct HasMutableDataAccess : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasMutableDataAccess type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct HasMutableDataAccess<const T> : public HasMutableDataAccess<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasMutableDataAccess type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct HasMutableDataAccess<volatile T> : public HasMutableDataAccess<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasMutableDataAccess type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct HasMutableDataAccess<const volatile T> : public HasMutableDataAccess<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasMutableDataAccess type trait.
// \ingroup math_type_traits
//
// The HasMutableDataAccess_v variable template provides a convenient shortcut to access the nested
// \a value of the HasMutableDataAccess class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasMutableDataAccess<T>::value;
   constexpr bool value2 = mtrc::numeric::HasMutableDataAccess_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasMutableDataAccess_v = HasMutableDataAccess<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
