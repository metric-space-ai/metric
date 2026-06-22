// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISENUM_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISENUM_H
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
/*!\brief Compile time check for complete enumeration types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is an enumeration type.
// In case the type is an enumeration type, the \a value member constant is set to \a true,
// the nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.

   \code
   enum A {};
   enum B : int {};
   enum class C {};
   class D {};

   mtrc::numeric::IsEnum<A>::value       // Evaluates to 'true'
   mtrc::numeric::IsEnum<const B>::Type  // Results in TrueType
   mtrc::numeric::IsEnum<volatile C>     // Is derived from TrueType
   mtrc::numeric::IsEnum<int>::value     // Evaluates to 'false'
   mtrc::numeric::IsEnum<double>::Type   // Results in FalseType
   mtrc::numeric::IsEnum<D>              // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsEnum : public BoolConstant<std::is_enum<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsEnum type trait.
// \ingroup type_traits
//
// The IsEnum_v variable template provides a convenient shortcut to access the nested \a value
// of the IsEnum class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsEnum<T>::value;
   constexpr bool value2 = mtrc::numeric::IsEnum_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsEnum_v = IsEnum<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
