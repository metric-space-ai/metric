// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISCUSTOM_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISCUSTOM_H
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
/*!\brief Compile time check for custom data types.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type is a custom data type, i.e. a custom vector
// or a custom matrix. In case the data type a custom data type, the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType. Examples:

   \code
   using mtrc::numeric::CustomVector;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::CustomMatrix;
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::aligned;
   using mtrc::numeric::unpadded;
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowMajor;

   using CustomVectorType = CustomVector<int,aligned,unpadded,columnVector>;
   using CustomMatrixType = CustomMatrix<double,aligned,unpadded,rowMajor>;

   mtrc::numeric::IsCustom< CustomVectorType >::value                        // Evaluates to 1
   mtrc::numeric::IsCustom< const CustomVectorType >::Type                   // Results in TrueType
   mtrc::numeric::IsCustom< volatile CustomMatrixType >                      // Is derived from TrueType
   mtrc::numeric::IsCustom< int >::value                                     // Evaluates to 0
   mtrc::numeric::IsCustom< const DynamicVector<float,columnVector> >::Type  // Results in FalseType
   mtrc::numeric::IsCustom< volatile DynamicMatrix<int,rowMajor> >           // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsCustom : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsCustom type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsCustom<const T> : public IsCustom<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsCustom type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsCustom<volatile T> : public IsCustom<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsCustom type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsCustom<const volatile T> : public IsCustom<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsCustom type trait.
// \ingroup math_type_traits
//
// The IsCustom_v variable template provides a convenient shortcut to access the nested \a value
// of the IsCustom class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsCustom<T>::value;
   constexpr bool value2 = mtrc::numeric::IsCustom_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsCustom_v = IsCustom<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
