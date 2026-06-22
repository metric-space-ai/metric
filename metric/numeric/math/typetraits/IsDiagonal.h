// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISDIAGONAL_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISDIAGONAL_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsLower.h>
#include <metric/numeric/math/typetraits/IsUpper.h>
#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for diagonal matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a diagonal matrix type
// (i.e. a matrix type that is guaranteed to be diagonal at compile time). In case the type is
// a diagonal matrix type, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using mtrc::numeric::rowMajor;

   using StaticMatrixType     = mtrc::numeric::StaticMatrix<double,3UL,3UL,rowMajor>;
   using DynamicMatrixType    = mtrc::numeric::DynamicMatrix<float,rowMajor>;
   using CompressedMatrixType = mtrc::numeric::CompressedMatrix<int,rowMajor>;

   using DiagonalStaticType     = mtrc::numeric::DiagonalMatrix<StaticMatrixType>;
   using DiagonalDynamicType    = mtrc::numeric::DiagonalMatrix<DynamicMatrixType>;
   using DiagonalCompressedType = mtrc::numeric::DiagonalMatrix<CompressedMatrixType>;

   using LowerStaticType  = mtrc::numeric::LowerMatrix<StaticMatrixType>;
   using UpperDynamicType = mtrc::numeric::UpperMatrix<DynamicMatrixType>;

   mtrc::numeric::IsDiagonal< DiagonalStaticType >::value           // Evaluates to 1
   mtrc::numeric::IsDiagonal< const DiagonalDynamicType >::Type     // Results in TrueType
   mtrc::numeric::IsDiagonal< volatile DiagonalCompressedType >     // Is derived from TrueType
   mtrc::numeric::IsDiagonal< LowerStaticMatrixType >::value        // Evaluates to 0
   mtrc::numeric::IsDiagonal< const UpperDynamicMatrixType >::Type  // Results in FalseType
   mtrc::numeric::IsDiagonal< volatile CompressedMatrixType >       // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsDiagonal : public BoolConstant<IsLower_v<T> && IsUpper_v<T>> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsDiagonal type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsDiagonal<const T> : public IsDiagonal<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsDiagonal type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsDiagonal<volatile T> : public IsDiagonal<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsDiagonal type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsDiagonal<const volatile T> : public IsDiagonal<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsDiagonal type trait.
// \ingroup math_type_traits
//
// The IsDiagonal_v variable template provides a convenient shortcut to access the nested
// \a value of the IsDiagonal class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsDiagonal<T>::value;
   constexpr bool value2 = mtrc::numeric::IsDiagonal_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsDiagonal_v = IsDiagonal<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
