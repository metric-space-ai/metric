// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSTATIC_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSTATIC_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsVector.h>
#include <metric/numeric/math/typetraits/Size.h>
#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for static data types.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type is a static data type, i.e. a vector or
// matrix with dimensions fixed at compile time. In case the data type a static data type, the
// \a value member constant is set to \a true, the nested type definition \a Type is \a TrueType,
// and the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType. Examples:

   \code
   using mtrc::numeric::StaticVector;
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowMajor;

   using StaticVectorType = StaticVector<int,columnVector>;
   using StaticMatrixType = StaticMatrix<double,rowMajor>;

   mtrc::numeric::IsStatic< StaticVectorType >::value                        // Evaluates to 1
   mtrc::numeric::IsStatic< const StaticVectorType >::Type                   // Results in TrueType
   mtrc::numeric::IsStatic< volatile StaticMatrixType >                      // Is derived from TrueType
   mtrc::numeric::IsStatic< int >::value                                     // Evaluates to 0
   mtrc::numeric::IsStatic< const DynamicVector<float,columnVector> >::Type  // Results in FalseType
   mtrc::numeric::IsStatic< volatile DynamicMatrix<int,rowMajor> >           // Is derived from FalseType
   \endcode
*/
template <typename T>
struct IsStatic
	: public BoolConstant<(IsVector_v<T> && Size_v<T, 0UL> != DefaultSize_v) ||
						  (IsMatrix_v<T> && Size_v<T, 0UL> != DefaultSize_v && Size_v<T, 1UL> != DefaultSize_v)> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsStatic type trait.
// \ingroup math_type_traits
//
// The IsStatic_v variable template provides a convenient shortcut to access the nested \a value
// of the IsStatic class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsStatic<T>::value;
   constexpr bool value2 = mtrc::numeric::IsStatic_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsStatic_v = IsStatic<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
