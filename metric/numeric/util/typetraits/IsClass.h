// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISCLASS_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISCLASS_H
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
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsClass type trait tests whether or not the given template parameter is a (possibly
// cv-qualified) class type. In case the given data type is a class type, the \a value member
// constant is set to \a true, the nested type definition \a Type is set to \a TrueType, and
// the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType and the class derives from \a FalseType.

   \code
   class MyClass {};

   mtrc::numeric::IsClass<MyClass>::value        // Evaluates to 'true'
   mtrc::numeric::IsClass<MyClass const>::Type   // Results in TrueType
   mtrc::numeric::IsClass<std::string volatile>  // Is derived from TrueType
   mtrc::numeric::IsClass<int>::value            // Evaluates to 'false' (int is a built-in data type)
   mtrc::numeric::IsClass<MyClass&>::Type        // Results in FalseType
   mtrc::numeric::IsClass<MyClass*>              // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsClass : public BoolConstant<std::is_class<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsClass type trait.
// \ingroup type_traits
//
// The IsClass_v variable template provides a convenient shortcut to access the nested \a value
// of the IsClass class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsClass<T>::value;
   constexpr bool value2 = mtrc::numeric::IsClass_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsClass_v = IsClass<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
