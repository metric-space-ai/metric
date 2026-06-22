// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISUNITRIANGULAR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISUNITRIANGULAR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsUniLower.h>
#include <metric/numeric/math/typetraits/IsUniUpper.h>
#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for unitriangular matrix types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a lower or upper
// unitriangular matrix type. In case the type is an unitriangular matrix type, the \a value
// member constant is set to \a true, the nested type definition \a Type is \a TrueType,
// and the class derives from \a TrueType. Otherwise \a yes is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType.

   \code
   using mtrc::numeric::rowMajor;

   using StaticMatrixType     = mtrc::numeric::StaticMatrix<double,3UL,3UL,rowMajor>;
   using DynamicMatrixType    = mtrc::numeric::DynamicMatrix<float,rowMajor>;
   using CompressedMatrixType = mtrc::numeric::CompressedMatrix<int,rowMajor>;

   using UniLowerStaticType     = mtrc::numeric::UniLowerMatrix<StaticMatrixType>;
   using UniUpperDynamicType    = mtrc::numeric::UniUpperMatrix<DynamicMatrixType>;
   using UniLowerCompressedType = mtrc::numeric::UniLowerMatrix<CompressedMatrixType>;

   mtrc::numeric::IsUniTriangular< UniLowerStaticType >::value        // Evaluates to 1
   mtrc::numeric::IsUniTriangular< const UniUpperDynamicType >::Type  // Results in TrueType
   mtrc::numeric::IsUniTriangular< volatile UniLowerCompressedType >  // Is derived from TrueType
   mtrc::numeric::IsUniTriangular< StaticMatrixType >::value          // Evaluates to 0
   mtrc::numeric::IsUniTriangular< const DynamicMatrixType >::Type    // Results in FalseType
   mtrc::numeric::IsUniTriangular< volatile CompressedMatrixType >    // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsUniTriangular : public BoolConstant<IsUniLower_v<T> || IsUniUpper_v<T>> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsUniTriangular type trait.
// \ingroup math_type_traits
//
// The IsUniTriangular_v variable template provides a convenient shortcut to access the nested
// \a value of the IsUniTriangular class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsUniTriangular<T>::value;
   constexpr bool value2 = mtrc::numeric::IsUniTriangular_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsUniTriangular_v = IsUniTriangular<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
