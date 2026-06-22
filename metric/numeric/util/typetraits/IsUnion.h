// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISUNION_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISUNION_H
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
/*!\brief Compile time check for union data types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is a union data type.
// In case the type is a union, the \a value member constant is set o \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   union A {
	  // ...
   };

   mtrc::numeric::IsUnion<A>::value       // Evaluates to 'true'
   mtrc::numeric::IsUnion<A const>::Type  // Results in TrueType
   mtrc::numeric::IsUnion<A volatile>     // Is derived from TrueType
   mtrc::numeric::IsUnion<int>::value     // Evaluates to 'false'
   mtrc::numeric::IsUnion<double>::Type   // Results in FalseType
   mtrc::numeric::IsUnion<std::string>    // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsUnion : public BoolConstant<std::is_union<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsUnion type trait.
// \ingroup type_traits
//
// The IsUnion_v variable template provides a convenient shortcut to access the nested \a value
// of the IsUnion class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsUnion<T>::value;
   constexpr bool value2 = mtrc::numeric::IsUnion_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsUnion_v = IsUnion<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
