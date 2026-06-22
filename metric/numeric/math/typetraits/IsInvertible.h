// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISINVERTIBLE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISINVERTIBLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsBLASCompatible.h>
#include <metric/numeric/math/typetraits/IsDenseMatrix.h>
#include <metric/numeric/math/typetraits/IsScalar.h>
#include <metric/numeric/math/typetraits/UnderlyingElement.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsFloatingPoint.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for data types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is invertible. The
// type is considered to be invertible if it is a floating point type (\c float, \c double,
// or <tt>long double</tt>), any other scalar type with a floating point element type (e.g.
// \c complex<float>, \c complex<double> or <tt>complex<long double></tt>) or any dense matrix
// type with a BLAS compatible element type. If the given type is invertible, the \a value
// member constant is set to \a true, the nested type definition \a Type is \a TrueType, and
// the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsInvertible< float >::value                  // Evaluates to 1
   mtrc::numeric::IsInvertible< complex<double> >::Type         // Results in TrueType
   mtrc::numeric::IsInvertible< mtrc::numeric::DynamicMatrix<double> >  // Is derived from TrueType
   mtrc::numeric::IsInvertible< int >::value                    // Evaluates to 0
   mtrc::numeric::IsInvertible< complex<int> >::Type            // Results in FalseType
   mtrc::numeric::IsInvertible< mtrc::numeric::DynamicVector<double> >  // Is derived from FalseType
   \endcode
*/
template <typename T>
struct IsInvertible : public BoolConstant<(IsScalar_v<T> && IsFloatingPoint_v<UnderlyingElement_t<T>>) ||
										  (IsDenseMatrix_v<T> && IsBLASCompatible_v<UnderlyingElement_t<T>>)> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsInvertible type trait.
// \ingroup math_type_traits
//
// The IsInvertible_v variable template provides a convenient shortcut to access the nested
// \a value of the IsInvertible class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsInvertible<T>::value;
   constexpr bool value2 = mtrc::numeric::IsInvertible_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsInvertible_v = IsInvertible<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
