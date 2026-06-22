// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSTRICTLYTRIANGULAR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSTRICTLYTRIANGULAR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsStrictlyLower.h>
#include <metric/numeric/math/typetraits/IsStrictlyUpper.h>
#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for strictly triangular matrix types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a strictly lower or
// upper triangular matrix type. In case the type is a triangular matrix type, the \a value
// member constant is set to \a true, the nested type definition \a Type is \a TrueType,
// and the class derives from \a TrueType. Otherwise \a yes is set to \a false, \a Type
// is \a FalseType, and the class derives from \a FalseType.

   \code
   using mtrc::numeric::rowMajor;

   using StaticMatrixType     = mtrc::numeric::StaticMatrix<double,3UL,3UL,rowMajor>;
   using DynamicMatrixType    = mtrc::numeric::DynamicMatrix<float,rowMajor>;
   using CompressedMatrixType = mtrc::numeric::CompressedMatrix<int,rowMajor>;

   using StrictlyLowerStaticType     = mtrc::numeric::StrictlyLowerMatrix<StaticMatrixType>;
   using StrictlyUpperDynamicType    = mtrc::numeric::StrictlyUpperMatrix<DynamicMatrixType>;
   using StrictlyLowerCompressedType = mtrc::numeric::StrictlyLowerMatrix<CompressedMatrixType>;

   mtrc::numeric::IsStrictlyTriangular< StrictlyLowerStaticType >::value        // Evaluates to 1
   mtrc::numeric::IsStrictlyTriangular< const StrictlyUpperDynamicType >::Type  // Results in TrueType
   mtrc::numeric::IsStrictlyTriangular< volatile StrictlyLowerCompressedType >  // Is derived from TrueType
   mtrc::numeric::IsStrictlyTriangular< StaticMatrixType >::value               // Evaluates to 0
   mtrc::numeric::IsStrictlyTriangular< const DynamicMatrixType >::Type         // Results in FalseType
   mtrc::numeric::IsStrictlyTriangular< volatile CompressedMatrixType >         // Is derived from FalseType
   \endcode
*/
template <typename T>
struct IsStrictlyTriangular : public BoolConstant<IsStrictlyLower_v<T> || IsStrictlyUpper_v<T>> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsStrictlyTriangular type trait.
// \ingroup math_type_traits
//
// The IsStrictlyTriangular_v variable template provides a convenient shortcut to access the
// nested \a value of the IsStrictlyTriangular class template. For instance, given the type
// \a T the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsStrictlyTriangular<T>::value;
   constexpr bool value2 = mtrc::numeric::IsStrictlyTriangular_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsStrictlyTriangular_v = IsStrictlyTriangular<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
