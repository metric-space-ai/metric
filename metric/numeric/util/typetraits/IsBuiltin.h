// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISBUILTIN_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISBUILTIN_H
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
/*!\brief Compile time check for built-in data types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is a built-in/fundamental
// data type. In case the type is a built-in type, the \a value member constant is set to \a true,
// the nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.

   \code
   mtrc::numeric::IsBuiltin<void>::value         // Evaluates to 'true'
   mtrc::numeric::IsBuiltin<float const>::Type   // Results in TrueType
   mtrc::numeric::IsBuiltin<short volatile>      // Is derived from TrueType
   mtrc::numeric::IsBuiltin<std::string>::value  // Evaluates to 'false'
   mtrc::numeric::IsBuiltin<int*>::Type          // Results in FalseType
   mtrc::numeric::IsBuiltin<int&>                // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsBuiltin : public BoolConstant<std::is_fundamental<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsBuiltin type trait.
// \ingroup type_traits
//
// The IsBuiltin_v variable template provides a convenient shortcut to access the nested
// \a value of the IsBuiltin class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsBuiltin<T>::value;
   constexpr bool value2 = mtrc::numeric::IsBuiltin_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsBuiltin_v = IsBuiltin<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
