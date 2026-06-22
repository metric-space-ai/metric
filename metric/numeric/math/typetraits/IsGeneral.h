// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISGENERAL_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISGENERAL_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsHermitian.h>
#include <metric/numeric/math/typetraits/IsSymmetric.h>
#include <metric/numeric/math/typetraits/IsTriangular.h>
#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for general matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a general matrix type
// (i.e. a matrix type that is neither symmetric, Hermitian, lower triangular or upper triangular
// at compile time). In case the type is a general matrix type, the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.

   \code
   using mtrc::numeric::rowMajor;

   using StaticMatrixType     = mtrc::numeric::StaticMatrix<double,3UL,3UL,rowMajor>;
   using DynamicMatrixType    = mtrc::numeric::DynamicMatrix<float,rowMajor>;
   using CompressedMatrixType = mtrc::numeric::CompressedMatrix<int,rowMajor>;

   using SymmetricStaticType  = mtrc::numeric::SymmetricMatrix<StaticMatrixType>;
   using HermitianDynamicType = mtrc::numeric::HermitianMatrix<DynamicMatrixType>;
   using LowerCompressedType  = mtrc::numeric::LowerMatrix<CompressedMatrixType>;

   mtrc::numeric::IsGeneral< StaticMatrixType >::value           // Evaluates to 1
   mtrc::numeric::IsGeneral< const DynamicMatrixType >::Type     // Results in TrueType
   mtrc::numeric::IsGeneral< volatile CompressedMatrixType >     // Is derived from TrueType
   mtrc::numeric::IsGeneral< SymmetricStaticType >::value        // Evaluates to 0
   mtrc::numeric::IsGeneral< const HermitianDynamicType >::Type  // Results in FalseType
   mtrc::numeric::IsGeneral< volatile LowerCompressedType >      // Is derived from FalseType
   \endcode
*/
template <typename T>
struct IsGeneral : public BoolConstant<!IsSymmetric_v<T> && !IsHermitian_v<T> && !IsTriangular_v<T>> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsGeneral type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsGeneral<const T> : public IsGeneral<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsGeneral type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsGeneral<volatile T> : public IsGeneral<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsGeneral type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsGeneral<const volatile T> : public IsGeneral<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsGeneral type trait.
// \ingroup math_type_traits
//
// The IsGeneral_v variable template provides a convenient shortcut to access the nested
// \a value of the IsGeneral class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsGeneral<T>::value;
   constexpr bool value2 = mtrc::numeric::IsGeneral_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsGeneral_v = IsGeneral<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
