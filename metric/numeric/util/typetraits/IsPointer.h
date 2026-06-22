// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISPOINTER_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISPOINTER_H
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
// This class tests whether the given template parameter is a pointer type (including function
// pointers, but excluding pointers to members) or not. If it is a pointer type, the \a value
// member constant is set to \a true, the nested type definition \a Type is \a TrueType, and
// the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsPointer<char* const>::value      // Evaluates to 'true'
   mtrc::numeric::IsPointer<volatile float*>::Type   // Results in TrueType
   mtrc::numeric::IsPointer<int (*)(long)>           // Is derived from TrueType
   mtrc::numeric::IsPointer<int>::value              // Evaluates to 'false'
   mtrc::numeric::IsPointer<int MyClass::*>::Type    // Results in FalseType
   mtrc::numeric::IsPointer<int (MyClass::*)(long)>  // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsPointer : public BoolConstant<std::is_pointer<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsPointer type trait.
// \ingroup type_traits
//
// The IsPointer_v variable template provides a convenient shortcut to access the nested
// \a value of the IsPointer class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsPointer<T>::value;
   constexpr bool value2 = mtrc::numeric::IsPointer_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsPointer_v = IsPointer<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
