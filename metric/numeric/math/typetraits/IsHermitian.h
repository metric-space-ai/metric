// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISHERMITIAN_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISHERMITIAN_H
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
template <typename T> struct IsHermitian;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the IsHermitian type trait.
// \ingroup math_traits
*/
template <typename T, typename = void> struct IsHermitianHelper : public FalseType {};

template <typename T> // Type of the operand
struct IsHermitianHelper<T, EnableIf_t<IsExpression_v<T> && !IsSame_v<T, typename T::ResultType>>>
	: public IsHermitian<typename T::ResultType>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for Hermitian matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is an Hermitian matrix type
// (i.e. a matrix type that is guaranteed to be Hermitian at compile time). In case the type is
// a Hermitian matrix type, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using mtrc::numeric::rowMajor;

   using StaticMatrixType     = mtrc::numeric::StaticMatrix<double,rowMajor>;
   using DynamicMatrixType    = mtrc::numeric::DynamicMatrix<float,rowMajor>;
   using CompressedMatrixType = mtrc::numeric::CompressedMatrix<int,rowMajor>;

   using HermitianStaticType     = mtrc::numeric::HermitianMatrix<StaticMatrixType>;
   using HermitianDynamicType    = mtrc::numeric::HermitianMatrix<DynamicMatrixType>;
   using HermitianCompressedType = mtrc::numeric::HermitianMatrix<CompressedMatrixType>;

   mtrc::numeric::IsHermitian< HermitianStaticType >::value        // Evaluates to 1
   mtrc::numeric::IsHermitian< const HermitianDynamicType >::Type  // Results in TrueType
   mtrc::numeric::IsHermitian< volatile HermitianCompressedType >  // Is derived from TrueType
   mtrc::numeric::IsHermitian< StaticMatrixType >::value           // Evaluates to 0
   mtrc::numeric::IsHermitian< const DynamicMatrixType >::Type     // Results in FalseType
   mtrc::numeric::IsHermitian< volatile CompressedMatrixType >     // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsHermitian : public IsHermitianHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsHermitian type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsHermitian<const T> : public IsHermitian<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsHermitian type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsHermitian<volatile T> : public IsHermitian<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsHermitian type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsHermitian<const volatile T> : public IsHermitian<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsHermitian type trait.
// \ingroup math_type_traits
//
// The IsHermitian_v variable template provides a convenient shortcut to access the nested
// \a value of the IsHermitian class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsHermitian<T>::value;
   constexpr bool value2 = mtrc::numeric::IsHermitian_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsHermitian_v = IsHermitian<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
