// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISLVALUEREFERENCE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISLVALUEREFERENCE_H
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
// This class tests whether the given template parameter \a T is an lvalue reference type. If it
// is an lvalue reference type, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   class A {};

   mtrc::numeric::IsLValueReference<int&>::value      // Evaluates to 'true'
   mtrc::numeric::IsLValueReference<const A&>::Type   // Results in TrueType
   mtrc::numeric::IsLValueReference<int (&)(int)>     // Is derived from TrueType
   mtrc::numeric::IsLValueReference<int>::value       // Evaluates to 'false'
   mtrc::numeric::IsLValueReference<const A&&>::Type  // Results in FalseType
   mtrc::numeric::IsLValueReference<int (A::*)(int)>  // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsLValueReference : public BoolConstant<std::is_lvalue_reference<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsLValueReference type trait.
// \ingroup type_traits
//
// The IsLValueReference_v variable template provides a convenient shortcut to access the nested
// \a value of the IsLValueReference class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsLValueReference<T>::value;
   constexpr bool value2 = mtrc::numeric::IsLValueReference_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsLValueReference_v = IsLValueReference<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
