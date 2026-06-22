// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISUPPER_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISUPPER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/typetraits/IsStrictlyUpper.h>
#include <metric/numeric/math/typetraits/IsUniUpper.h>
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
template <typename T> struct IsUpper;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the IsUpper type trait.
// \ingroup math_traits
*/
template <typename T, typename = void>
struct IsUpperHelper : public BoolConstant<IsUniUpper_v<T> || IsStrictlyUpper_v<T>> {};

template <typename T> // Type of the operand
struct IsUpperHelper<T, EnableIf_t<IsExpression_v<T> && !IsSame_v<T, typename T::ResultType>>>
	: public IsUpper<typename T::ResultType>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for upper triangular matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is an upper triangular matrix
// type (i.e. a matrix type that is guaranteed to be upper triangular at compile time). This also
// includes upper unitriangular and strictly upper triangular matrices. In case the type is an
// upper triangular matrix type, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using mtrc::numeric::rowMajor;

   using StaticMatrixType     = mtrc::numeric::StaticMatrix<double,3UL,3UL,rowMajor>;
   using DynamicMatrixType    = mtrc::numeric::DynamicMatrix<float,rowMajor>;
   using CompressedMatrixType = mtrc::numeric::CompressedMatrix<int,rowMajor>;

   using UpperStaticType        = mtrc::numeric::UpperMatrix<StaticMatrixType>;
   using UpperDynamicType       = mtrc::numeric::UpperMatrix<DynamicMatrixType>;
   using UniUpperCompressedType = mtrc::numeric::UniUpperMatrix<CompressedMatrixType>;

   mtrc::numeric::IsUpper< UpperStaticType >::value           // Evaluates to 1
   mtrc::numeric::IsUpper< const UpperDynamicType >::Type     // Results in TrueType
   mtrc::numeric::IsUpper< volatile UniUpperCompressedType >  // Is derived from TrueType
   mtrc::numeric::IsUpper< StaticMatrixType >::value          // Evaluates to 0
   mtrc::numeric::IsUpper< const DynamicMatrixType >::Type    // Results in FalseType
   mtrc::numeric::IsUpper< volatile CompressedMatrixType >    // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsUpper : public IsUpperHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsUpper type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsUpper<const T> : public IsUpper<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsUpper type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsUpper<volatile T> : public IsUpper<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsUpper type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsUpper<const volatile T> : public IsUpper<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsUpper type trait.
// \ingroup math_type_traits
//
// The IsUpper_v variable template provides a convenient shortcut to access the nested
// \a value of the IsUpper class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsUpper<T>::value;
   constexpr bool value2 = mtrc::numeric::IsUpper_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsUpper_v = IsUpper<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
