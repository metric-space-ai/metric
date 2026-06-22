// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISUNIUPPER_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISUNIUPPER_H
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
template <typename T> struct IsUniUpper;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the IsUniUpper type trait.
// \ingroup math_traits
*/
template <typename T, typename = void> struct IsUniUpperHelper : public FalseType {};

template <typename T> // Type of the operand
struct IsUniUpperHelper<T, EnableIf_t<IsExpression_v<T> && !IsSame_v<T, typename T::ResultType>>>
	: public IsUniUpper<typename T::ResultType>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for upper unitriangular matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is an upper unitriangular
// matrix type (i.e. a matrix type that is guaranteed to be upper unitriangular at compile time).
// In case the type is an upper unitriangular matrix type, the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.

   \code
   using mtrc::numeric::rowMajor;

   using StaticMatrixType     = mtrc::numeric::StaticMatrix<double,3UL,3UL,rowMajor>;
   using DynamicMatrixType    = mtrc::numeric::DynamicMatrix<float,rowMajor>;
   using CompressedMatrixType = mtrc::numeric::CompressedMatrix<int,rowMajor>;

   using UniUpperStaticType     = mtrc::numeric::UniUpperMatrix<StaticMatrixType>;
   using UniUpperDynamicType    = mtrc::numeric::UniUpperMatrix<DynamicMatrixType>;
   using UniUpperCompressedType = mtrc::numeric::UniUpperMatrix<CompressedMatrixType>;

   mtrc::numeric::IsUniUpper< UniUpperStaticType >::value        // Evaluates to 1
   mtrc::numeric::IsUniUpper< const UniUpperDynamicType >::Type  // Results in TrueType
   mtrc::numeric::IsUniUpper< volatile UniUpperCompressedType >  // Is derived from TrueType
   mtrc::numeric::IsUniUpper< StaticMatrixType >::value          // Evaluates to 0
   mtrc::numeric::IsUniUpper< const DynamicMatrixType >::Type    // Results in FalseType
   mtrc::numeric::IsUniUpper< volatile CompressedMatrixType >    // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsUniUpper : public IsUniUpperHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsUniUpper type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsUniUpper<const T> : public IsUniUpper<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsUniUpper type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsUniUpper<volatile T> : public IsUniUpper<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsUniUpper type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsUniUpper<const volatile T> : public IsUniUpper<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsUniUpper type trait.
// \ingroup math_type_traits
//
// The IsUniUpper_v variable template provides a convenient shortcut to access the nested
// \a value of the IsUniUpper class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsUniUpper<T>::value;
   constexpr bool value2 = mtrc::numeric::IsUniUpper_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsUniUpper_v = IsUniUpper<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
