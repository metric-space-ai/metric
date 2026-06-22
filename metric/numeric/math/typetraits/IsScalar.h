// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSCALAR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSCALAR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/HasCompositeType.h>
#include <metric/numeric/math/typetraits/IsProxy.h>
#include <metric/numeric/math/typetraits/IsSIMDPack.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsPointer.h>
#include <metric/numeric/util/typetraits/IsReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for scalar types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is scalar type (i.e. neither
// a vector, nor a matrix, proxy, SIMD pack, reference or pointer type). In case the type is a
// scalar type, the \a value member constant is set to \a true, the nested type definition \a Type
// is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsScalar< int >::value                         // Evaluates to 1
   mtrc::numeric::IsScalar< const double >::Type                 // Results in TrueType
   mtrc::numeric::IsScalar< volatile complex<float> >            // Is derived from TrueType
   mtrc::numeric::IsScalar< StaticVector<double,3UL> >::value    // Evaluates to 0
   mtrc::numeric::IsScalar< const DynamicMatrix<double> >::Type  // Results in FalseType
   mtrc::numeric::IsScalar< volatile CompressedMatrix<int> >     // Is derived from FalseType
   \endcode
*/

template <typename T>
struct IsScalar : public BoolConstant<!HasCompositeType_v<T> && !IsProxy_v<T> && !IsSIMDPack_v<T> &&
									  !IsReference_v<T> && !IsPointer_v<T>> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsScalar type trait.
// \ingroup math_type_traits
//
// The IsScalar_v variable template provides a convenient shortcut to access the nested \a value
// of the IsScalar class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsScalar<T>::value;
   constexpr bool value2 = mtrc::numeric::IsScalar_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsScalar_v = IsScalar<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
