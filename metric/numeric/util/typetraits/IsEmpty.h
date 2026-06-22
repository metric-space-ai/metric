// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISEMPTY_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISEMPTY_H
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
// This class tests whether the given template parameter is an empty class type, i.e. a type
// without member data and virtual functions. If it is an empty class type, the \a value
// member constant is set to \a true, the nested type definition \a Type is \a TrueType,
// and the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type
// is \a FalseType, and the class derives from \a FalseType.

   \code
   class A {};
   class B { int i; };

   mtrc::numeric::IsEmpty<A>::value           // Evaluates to 'true'
   mtrc::numeric::IsEmpty<A volatile>::Type   // Results in TrueType
   mtrc::numeric::IsEmpty<A const>            // Is derived from TrueType
   mtrc::numeric::IsEmpty<int>::value         // Evaluates to 'false'
   mtrc::numeric::IsEmpty<std::string>::Type  // Results in FalseType
   mtrc::numeric::IsEmpty<B>                  // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsEmpty : public BoolConstant<std::is_empty<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsEmpty type trait.
// \ingroup type_traits
//
// The IsEmpty_v variable template provides a convenient shortcut to access the nested \a value
// of the IsEmpty class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsEmpty<T>::value;
   constexpr bool value2 = mtrc::numeric::IsEmpty_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsEmpty_v = IsEmpty<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
