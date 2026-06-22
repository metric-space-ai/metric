// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISOBJECT_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISOBJECT_H
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
// This class tests whether the given template parameter \a T is an object type. All types are
// considered object types except references, \a void, and function types. If \a T is an object
// type, the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsObject<int>::value                   // Evaluates to 'true'
   mtrc::numeric::IsObject<int*>::Type                   // Results in TrueType
   mtrc::numeric::IsObject<int (*)(void)>                // Is derived from TrueType
   mtrc::numeric::IsObject<int (MyClass::*)(void)const>  // Also derived from TrueType
   mtrc::numeric::IsObject<int&>::value                  // Evaluates to 'false'
   mtrc::numeric::IsObject<const void>::Type             // Results in FalseType
   mtrc::numeric::IsObject<int (double)>                 // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsObject : public BoolConstant<std::is_object<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsObject type trait.
// \ingroup type_traits
//
// The IsObject_v variable template provides a convenient shortcut to access the nested \a value
// of the IsObject class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsObject<T>::value;
   constexpr bool value2 = mtrc::numeric::IsObject_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsObject_v = IsObject<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
