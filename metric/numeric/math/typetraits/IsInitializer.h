// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISINITIALIZER_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISINITIALIZER_H
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
// This type trait tests whether the given data type represents an initializer list, i.e. is an
// initializer vector or an initializer matrix. In case the data type represents an initializer
// list, the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType. Examples:

   \code
   using mtrc::numeric::InitializerVector;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::InitializerMatrix;
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowMajor;

   mtrc::numeric::IsInitializer< InitializerVector<int,columnVector> >::value     // Evaluates to 1
   mtrc::numeric::IsInitializer< const InitializerVector<int,rowVector> >::Type   // Results in TrueType
   mtrc::numeric::IsInitializer< volatile InitializerMatrix<double,rowMajor> >    // Is derived from TrueType
   mtrc::numeric::IsInitializer< int >::value                                     // Evaluates to 0
   mtrc::numeric::IsInitializer< const DynamicVector<float,columnVector> >::Type  // Results in FalseType
   mtrc::numeric::IsInitializer< volatile DynamicMatrix<int,rowMajor> >           // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsInitializer : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsInitializer type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsInitializer<const T> : public IsInitializer<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsInitializer type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsInitializer<volatile T> : public IsInitializer<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsInitializer type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsInitializer<const volatile T> : public IsInitializer<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsInitializer type trait.
// \ingroup math_type_traits
//
// The IsInitializer_v variable template provides a convenient shortcut to access the nested
// \a value of the IsInitializer class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsInitializer<T>::value;
   constexpr bool value2 = mtrc::numeric::IsInitializer_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsInitializer_v = IsInitializer<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
