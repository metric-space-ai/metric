// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSQUARE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSQUARE_H
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
/*!\brief Compile time check for square matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a square matrix type
// (i.e. a matrix type that is guaranteed to be square at compile time). In case the type is
// a square matrix type, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.

   \code
   using mtrc::numeric::rowMajor;

   // Type definitions of square matrix types
   using Mat2x2 = mtrc::numeric::StaticMatrix<double,2UL,2UL,rowMajor>;
   using Mat3x3 = mtrc::numeric::StaticMatrix<double,3UL,3UL,rowMajor>;
   using Mat4x4 = mtrc::numeric::StaticMatrix<double,4UL,4UL,rowMajor>;

   // Type definitions of non-square matrix types
   using Mat2x3            = mtrc::numeric::StaticMatrix<double,2UL,3UL,rowMajor>;
   using DynamicMatrixType = mtrc::numeric::DynamicMatrix<double,rowMajor>;
   using HybridMatrixType  = mtrc::numeric::HybridMatrix<double,3UL,3UL,rowMajor>;

   mtrc::numeric::IsSquare< Mat2x2 >::value              // Evaluates to 1
   mtrc::numeric::IsSquare< const Mat3x3 >::Type         // Results in TrueType
   mtrc::numeric::IsSquare< volatile Mat4x4 >            // Is derived from TrueType
   mtrc::numeric::IsSquare< DynamicMatrixType >::value   // Evaluates to 0
   mtrc::numeric::IsSquare< const Mat2x3 >::Type         // Results in FalseType
   mtrc::numeric::IsSquare< volatile HybridMatrixType >  // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsSquare : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSquare type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsSquare<const T> : public IsSquare<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSquare type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsSquare<volatile T> : public IsSquare<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSquare type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsSquare<const volatile T> : public IsSquare<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSquare type trait.
// \ingroup math_type_traits
//
// The IsSquare_v variable template provides a convenient shortcut to access the nested \a value
// of the IsSquare class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSquare<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSquare_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsSquare_v = IsSquare<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
