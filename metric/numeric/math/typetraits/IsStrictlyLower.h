// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSTRICTLYLOWER_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSTRICTLYLOWER_H
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
template <typename T> struct IsStrictlyLower;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the IsStrictlyLower type trait.
// \ingroup math_traits
*/
template <typename T, typename = void> struct IsStrictlyLowerHelper : public FalseType {};

template <typename T> // Type of the operand
struct IsStrictlyLowerHelper<T, EnableIf_t<IsExpression_v<T> && !IsSame_v<T, typename T::ResultType>>>
	: public IsStrictlyLower<typename T::ResultType>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for strictly lower triangular matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a strictly lower
// triangular matrix type (i.e. a matrix type that is guaranteed to be strictly lower
// triangular at compile time). In case the type is a strictly lower triangular matrix
// type, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using mtrc::numeric::rowMajor;

   using StaticMatrixType     = mtrc::numeric::StaticMatrix<double,3UL,3UL,rowMajor>;
   using DynamicMatrixType    = mtrc::numeric::DynamicMatrix<float,rowMajor>;
   using CompressedMatrixType = mtrc::numeric::CompressedMatrix<int,rowMajor>;

   using StrictlyLowerStaticType     = mtrc::numeric::StrictlyLowerMatrix<StaticMatrixType>;
   using StrictlyLowerDynamicType    = mtrc::numeric::StrictlyLowerMatrix<DynamicMatrixType>;
   using StrictlyLowerCompressedType = mtrc::numeric::StrictlyLowerMatrix<CompressedMatrixType>;

   mtrc::numeric::IsStrictlyLower< StrictlyLowerStaticType >::value        // Evaluates to 1
   mtrc::numeric::IsStrictlyLower< const StrictlyLowerDynamicType >::Type  // Results in TrueType
   mtrc::numeric::IsStrictlyLower< volatile StrictlyLowerCompressedType >  // Is derived from TrueType
   mtrc::numeric::IsStrictlyLower< StaticMatrixType >::value               // Evaluates to 0
   mtrc::numeric::IsStrictlyLower< const DynamicMatrixType >::Type         // Results in FalseType
   mtrc::numeric::IsStrictlyLower< volatile CompressedMatrixType >         // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsStrictlyLower : public IsStrictlyLowerHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsStrictlyLower type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsStrictlyLower<const T> : public IsStrictlyLower<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsStrictlyLower type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsStrictlyLower<volatile T> : public IsStrictlyLower<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsStrictlyLower type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsStrictlyLower<const volatile T> : public IsStrictlyLower<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsStrictlyLower type trait.
// \ingroup math_type_traits
//
// The IsStrictlyLower_v variable template provides a convenient shortcut to access the nested
// \a value of the IsStrictlyLower class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsStrictlyLower<T>::value;
   constexpr bool value2 = mtrc::numeric::IsStrictlyLower_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsStrictlyLower_v = IsStrictlyLower<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
