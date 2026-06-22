// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISTRIANGULAR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISTRIANGULAR_H
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
/*!\brief Compile time check for triangular matrix types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a lower or upper triangular
// matrix type. In case the type is a triangular matrix type, the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a yes is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.

   \code
   using mtrc::numeric::rowMajor;

   using StaticMatrixType     = mtrc::numeric::StaticMatrix<double,3UL,3UL,rowMajor>;
   using DynamicMatrixType    = mtrc::numeric::DynamicMatrix<float,rowMajor>;
   using CompressedMatrixType = mtrc::numeric::CompressedMatrix<int,rowMajor>;

   using LowerStaticType     = mtrc::numeric::LowerMatrix<StaticMatrixType>;
   using UpperDynamicType    = mtrc::numeric::UpperMatrix<DynamicMatrixType>;
   using LowerCompressedType = mtrc::numeric::LowerMatrix<CompressedMatrixType>;

   mtrc::numeric::IsLower< LowerStaticType >::value         // Evaluates to 1
   mtrc::numeric::IsLower< const UpperDynamicType >::Type   // Results in TrueType
   mtrc::numeric::IsLower< volatile LowerCompressedType >   // Is derived from TrueType
   mtrc::numeric::IsLower< StaticMatrixType >::value        // Evaluates to 0
   mtrc::numeric::IsLower< const DynamicMatrixType >::Type  // Results in FalseType
   mtrc::numeric::IsLower< volatile CompressedMatrixType >  // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsTriangular : public BoolConstant<IsLower_v<T> || IsUpper_v<T>> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsTriangular type trait.
// \ingroup math_type_traits
//
// The IsTriangular_v variable template provides a convenient shortcut to access the nested
// \a value of the IsTriangular class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsTriangular<T>::value;
   constexpr bool value2 = mtrc::numeric::IsTriangular_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsTriangular_v = IsTriangular<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
