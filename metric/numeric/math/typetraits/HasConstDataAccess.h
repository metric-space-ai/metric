// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASCONSTDATAACCESS_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASCONSTDATAACCESS_H
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
/*!\brief Compile time check for low-level access to constant data.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type provides a low-level access to constant
// data via a const 'data' member function. In case the according member function is provided,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType. Examples:

   \code
   mtrc::numeric::HasConstDataAccess< StaticVector<float,3U> >::value      // Evaluates to 1
   mtrc::numeric::HasConstDataAccess< const DynamicVector<double> >::Type  // Results in TrueType
   mtrc::numeric::HasConstDataAccess< volatile DynamicMatrix<int> >        // Is derived from TrueType
   mtrc::numeric::HasConstDataAccess< int >::value                         // Evaluates to 0
   mtrc::numeric::HasConstDataAccess< const CompressedVector<int> >::Type  // Results in FalseType
   mtrc::numeric::HasConstDataAccess< volatile CompressedMatrix<int> >     // Is derived from FalseType
   \endcode
*/
template <typename T> struct HasConstDataAccess : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasConstDataAccess type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct HasConstDataAccess<const T> : public HasConstDataAccess<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasConstDataAccess type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct HasConstDataAccess<volatile T> : public HasConstDataAccess<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasConstDataAccess type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct HasConstDataAccess<const volatile T> : public HasConstDataAccess<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasConstDataAccess type trait.
// \ingroup math_type_traits
//
// The HasConstDataAccess_v variable template provides a convenient shortcut to access the nested
// \a value of the HasConstDataAccess class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasConstDataAccess<T>::value;
   constexpr bool value2 = mtrc::numeric::HasConstDataAccess_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasConstDataAccess_v = HasConstDataAccess<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
