// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISFLOATINGPOINT_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISFLOATINGPOINT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <type_traits>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for floating point data types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is a floating point
// data type (ignoring the cv-qualifiers). In case the type is a floating point data type,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsFloatingPoint<float>::value            // Evaluates to 'true'
   mtrc::numeric::IsFloatingPoint<volatile double>::Type   // Result in TrueType
   mtrc::numeric::IsFloatingPoint<const long double>       // Is derived from TrueType
   mtrc::numeric::IsFloatingPoint<int>::value              // Evaluates to 'false'
   mtrc::numeric::IsFloatingPoint<const short>::Type       // Results in FalseType
   mtrc::numeric::IsFloatingPoint<volatile wchar_t>        // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsFloatingPoint : public BoolConstant<std::is_floating_point<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsFloatingPoint type trait.
// \ingroup type_traits
//
// The IsFloatingPoint_v variable template provides a convenient shortcut to access the nested
// \a value of the IsFloatingPoint class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsFloatingPoint<T>::value;
   constexpr bool value2 = mtrc::numeric::IsFloatingPoint_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsFloatingPoint_v = IsFloatingPoint<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
