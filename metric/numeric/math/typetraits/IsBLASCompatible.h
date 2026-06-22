// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISBLASCOMPATIBLE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISBLASCOMPATIBLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsComplexDouble.h>
#include <metric/numeric/util/typetraits/IsComplexFloat.h>
#include <metric/numeric/util/typetraits/IsDouble.h>
#include <metric/numeric/util/typetraits/IsFloat.h>

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
// This type trait tests whether or not the given template parameter is a data type compatible
// to the BLAS standard. The BLAS standard supports \c float, \c double, \c complex<float> and
// \c complex<double> values. If the type is BLAS compatible, the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives
// from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the
// class derives from \a FalseType.

   \code
   mtrc::numeric::IsBLASCompatible< float >::value         // Evaluates to 1
   mtrc::numeric::IsBLASCompatible< double >::Type         // Results in TrueType
   mtrc::numeric::IsBLASCompatible< complex<float> >       // Is derived from TrueType
   mtrc::numeric::IsBLASCompatible< int >::value           // Evaluates to 0
   mtrc::numeric::IsBLASCompatible< unsigned long >::Type  // Results in FalseType
   mtrc::numeric::IsBLASCompatible< long double >          // Is derived from FalseType
   \endcode
*/
template <typename T>
struct IsBLASCompatible
	: public BoolConstant<IsFloat_v<T> || IsDouble_v<T> || IsComplexFloat_v<T> || IsComplexDouble_v<T>> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsBLASCompatible type trait.
// \ingroup math_type_traits
//
// The IsBLASCompatible_v variable template provides a convenient shortcut to access the nested
// \a value of the IsBLASCompatible class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsBLASCompatible<T>::value;
   constexpr bool value2 = mtrc::numeric::IsBLASCompatible_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsBLASCompatible_v = IsBLASCompatible<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
