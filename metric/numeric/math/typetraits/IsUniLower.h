// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISUNILOWER_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISUNILOWER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsSame.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> struct IsUniLower;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the IsUniLower type trait.
// \ingroup math_traits
*/
template <typename T, typename = void> struct IsUniLowerHelper : public FalseType {};

template <typename T> // Type of the operand
struct IsUniLowerHelper<T, EnableIf_t<IsExpression_v<T> && !IsSame_v<T, typename T::ResultType>>>
	: public IsUniLower<typename T::ResultType>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for lower unitriangular matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a lower unitriangular
// matrix type (i.e. a matrix type that is guaranteed to be lower unitriangular at compile time).
// In case the type is a lower unitriangular matrix type, the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.

   \code
   using mtrc::numeric::rowMajor;

   using StaticMatrixType     = mtrc::numeric::StaticMatrix<double,3UL,3UL,rowMajor>;
   using DynamicMatrixType    = mtrc::numeric::DynamicMatrix<float,rowMajor>;
   using CompressedMatrixType = mtrc::numeric::CompressedMatrix<int,rowMajor>;

   using UniLowerStaticType     = mtrc::numeric::UniLowerMatrix<StaticMatrixType>;
   using UniLowerDynamicType    = mtrc::numeric::UniLowerMatrix<DynamicMatrixType>;
   using UniLowerCompressedType = mtrc::numeric::UniLowerMatrix<CompressedMatrixType>;

   mtrc::numeric::IsUniLower< UniLowerStaticType >::value        // Evaluates to 1
   mtrc::numeric::IsUniLower< const UniLowerDynamicType >::Type  // Results in TrueType
   mtrc::numeric::IsUniLower< volatile UniLowerCompressedType >  // Is derived from TrueType
   mtrc::numeric::IsUniLower< StaticMatrixType >::value          // Evaluates to 0
   mtrc::numeric::IsUniLower< const DynamicMatrixType >::Type    // Results in FalseType
   mtrc::numeric::IsUniLower< volatile CompressedMatrixType >    // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsUniLower : public IsUniLowerHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsUniLower type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsUniLower<const T> : public IsUniLower<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsUniLower type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsUniLower<volatile T> : public IsUniLower<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsUniLower type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsUniLower<const volatile T> : public IsUniLower<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsUniLower type trait.
// \ingroup math_type_traits
//
// The IsUniLower_v variable template provides a convenient shortcut to access the nested
// \a value of the IsUniLower class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsUniLower<T>::value;
   constexpr bool value2 = mtrc::numeric::IsUniLower_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsUniLower_v = IsUniLower<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
