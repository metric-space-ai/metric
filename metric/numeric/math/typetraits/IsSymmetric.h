// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSYMMETRIC_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSYMMETRIC_H
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
template <typename T> struct IsSymmetric;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the IsSymmetric type trait.
// \ingroup math_traits
*/
template <typename T, typename = void> struct IsSymmetricHelper : public FalseType {};

template <typename T> // Type of the operand
struct IsSymmetricHelper<T, EnableIf_t<IsExpression_v<T> && !IsSame_v<T, typename T::ResultType>>>
	: public IsSymmetric<typename T::ResultType>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for symmetric matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a symmetric matrix type
// (i.e. a matrix type that is guaranteed to be symmetric at compile time). In case the type is
// a symmetric matrix type, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using mtrc::numeric::rowMajor;

   using StaticMatrixType     = mtrc::numeric::StaticMatrix<double,rowMajor>;
   using DynamicMatrixType    = mtrc::numeric::DynamicMatrix<float,rowMajor>;
   using CompressedMatrixType = mtrc::numeric::CompressedMatrix<int,rowMajor>;

   using SymmetricStaticType     = mtrc::numeric::SymmetricMatrix<StaticMatrixType>;
   using SymmetricDynamicType    = mtrc::numeric::SymmetricMatrix<DynamicMatrixType>;
   using SymmetricCompressedType = mtrc::numeric::SymmetricMatrix<CompressedMatrixType>;

   mtrc::numeric::IsSymmetric< SymmetricStaticType >::value        // Evaluates to 1
   mtrc::numeric::IsSymmetric< const SymmetricDynamicType >::Type  // Results in TrueType
   mtrc::numeric::IsSymmetric< volatile SymmetricCompressedType >  // Is derived from TrueType
   mtrc::numeric::IsSymmetric< StaticMatrixType >::value           // Evaluates to 0
   mtrc::numeric::IsSymmetric< const DynamicMatrixType >::Type     // Results in FalseType
   mtrc::numeric::IsSymmetric< volatile CompressedMatrixType >     // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsSymmetric : public IsSymmetricHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSymmetric type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsSymmetric<const T> : public IsSymmetric<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSymmetric type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsSymmetric<volatile T> : public IsSymmetric<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSymmetric type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsSymmetric<const volatile T> : public IsSymmetric<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSymmetric type trait.
// \ingroup math_type_traits
//
// The IsSymmetric_v variable template provides a convenient shortcut to access the nested
// \a value of the IsSymmetric class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSymmetric<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSymmetric_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsSymmetric_v = IsSymmetric<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
