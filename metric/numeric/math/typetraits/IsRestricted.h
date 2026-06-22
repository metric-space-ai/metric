// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISRESTRICTED_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISRESTRICTED_H
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
/*!\brief Compile time check for data types with restricted data access.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type has a restricted data access. Examples are
// the LowerMatrix and UpperMatrix adaptors that don't allow write access to the elements in the
// upper or lower part of the matrix, respectively. In case the data type has a restricted data
// access, the \a value member constant is set to \a true, the nested type definition \a Type
// is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType. Examples:

   \code
   using VectorType = mtrc::numeric::StaticVector<int,3UL>;
   using MatrixType = mtrc::numeric::DynamicMatrix<double>;

   using Lower = mtrc::numeric::LowerMatrix< mtrc::numeric::DynamicMatrix<double> >;
   using Upper = mtrc::numeric::LowerMatrix< mtrc::numeric::CompressedMatrix<int> >;

   mtrc::numeric::IsRestricted< Lower >::value            // Evaluates to 1
   mtrc::numeric::IsRestricted< const Upper >::Type       // Results in TrueType
   mtrc::numeric::IsRestricted< volatile Lower >          // Is derived from TrueType
   mtrc::numeric::IsRestricted< int >::value              // Evaluates to 0
   mtrc::numeric::IsRestricted< const VectorType >::Type  // Results in FalseType
   mtrc::numeric::IsRestricted< volatile MatrixType >     // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsRestricted : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsRestricted type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsRestricted<const T> : public IsRestricted<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsRestricted type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsRestricted<volatile T> : public IsRestricted<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsRestricted type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsRestricted<const volatile T> : public IsRestricted<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsRestricted type trait.
// \ingroup math_type_traits
//
// The IsRestricted_v variable template provides a convenient shortcut to access the nested
// \a value of the IsRestricted class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsRestricted<T>::value;
   constexpr bool value2 = mtrc::numeric::IsRestricted_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsRestricted_v = IsRestricted<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
