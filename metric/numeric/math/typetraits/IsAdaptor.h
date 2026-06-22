// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISADAPTOR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISADAPTOR_H
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
/*!\brief Compile time check for adaptors.
// \ingroup math_type_traits
//
// This type trait tests whether the given template parameter is an adaptor type (for instance
// \a LowerMatrix, \a UpperMatrix, or \a SymmetricMatrix). In case the type is an adaptor type,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType. The following example
// demonstrates this by means of the mentioned matrix adaptors:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::IsAdaptor

   using StaticMatrixType     = mtrc::numeric::StaticMatrix<double,3UL,3UL,rowMajor>;
   using DynamicMatrixType    = mtrc::numeric::DynamicMatrix<float,rowMajor>;
   using CompressedMatrixType = mtrc::numeric::CompressedMatrix<int,rowMajor>;

   using LowerStaticType         = mtrc::numeric::LowerMatrix<StaticMatrixType>;
   using UpperDynamicType        = mtrc::numeric::UpperMatrix<DynamicMatrixType>;
   using SymmetricCompressedType = mtrc::numeric::SymmetricMatrix<CompressedMatrixType>;

   mtrc::numeric::IsAdaptor< LowerStaticType >::value            // Evaluates to 1
   mtrc::numeric::IsAdaptor< const UpperDynamicType >::Type      // Results in TrueType
   mtrc::numeric::IsAdaptor< volatile SymmetricCompressedType >  // Is derived from TrueType
   mtrc::numeric::IsAdaptor< StaticMatrixType >::value           // Evaluates to 0
   mtrc::numeric::IsAdaptor< const DynamicMatrixType >::Type     // Results in FalseType
   mtrc::numeric::IsAdaptor< volatile CompressedMatrixType >     // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsAdaptor : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsAdaptor type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsAdaptor<const T> : public IsAdaptor<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsAdaptor type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsAdaptor<volatile T> : public IsAdaptor<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsAdaptor type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsAdaptor<const volatile T> : public IsAdaptor<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsAdaptor type trait.
// \ingroup math_type_traits
//
// The IsAdaptor_v variable template provides a convenient shortcut to access the nested
// \a value of the IsAdaptor class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsAdaptor<T>::value;
   constexpr bool value2 = mtrc::numeric::IsAdaptor_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsAdaptor_v = IsAdaptor<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
