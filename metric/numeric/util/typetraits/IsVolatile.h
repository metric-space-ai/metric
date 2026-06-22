// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISVOLATILE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISVOLATILE_H
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
/*!\brief Compile time check for volatile data types.
// \ingroup type_traits
//
// The IsVolatile type trait tests whether or not the given template parameter is a (top level)
// volatile-qualified data type. In case the given data type is volatile, the \a value member
// constant is set to \a true, the nested type definition \a Type is \a TrueType, and the class
// derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType,
// and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsVolatile<volatile int>::value        // Evaluates to 'true'
   mtrc::numeric::IsVolatile<const volatile int>::Type   // Results in TrueType
   mtrc::numeric::IsVolatile<int* volatile>              // Is derived from TrueType
   mtrc::numeric::IsVolatile<volatile int*>::value       // Evaluates to 'false' (the volatile qualifier is not at the
top level) mtrc::numeric::IsVolatile<const int>::Type            // Results in FalseType
   mtrc::numeric::IsVolatile<int>                        // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsVolatile : public BoolConstant<std::is_volatile<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsVolatile type trait.
// \ingroup type_traits
//
// The IsVolatile_v variable template provides a convenient shortcut to access the nested
// \a value of the IsVolatile class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsVolatile<T>::value;
   constexpr bool value2 = mtrc::numeric::IsVolatile_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsVolatile_v = IsVolatile<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
