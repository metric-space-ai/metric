// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISREFERENCE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISREFERENCE_H
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
// This class tests whether the given template parameter \a T is a reference type (including
// references to functions). If it is a reference type, the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.

   \code
   mtrc::numeric::IsReference<int&>::value             // Evaluates to 'true'
   mtrc::numeric::IsReference<int const&>::Type        // Results in TrueType
   mtrc::numeric::IsReference<int (&)(long)>           // Is derived from TrueType
   mtrc::numeric::IsReference<int>::value              // Evaluates to 'false'
   mtrc::numeric::IsReference<double*>::Type           // Results in FalseType
   mtrc::numeric::IsReference<int (MyClass::*)(long)>  // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsReference : public BoolConstant<std::is_reference<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsReference type trait.
// \ingroup type_traits
//
// The IsReference_v variable template provides a convenient shortcut to access the nested
// \a value of the IsReference class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsReference<T>::value;
   constexpr bool value2 = mtrc::numeric::IsReference_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsReference_v = IsReference<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
