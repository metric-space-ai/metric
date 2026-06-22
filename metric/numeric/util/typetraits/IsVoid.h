// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISVOID_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISVOID_H
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
/*!\brief Compile time check for the \a void data type.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is of type \a void
// (ignoring the cv-qualifiers). In case the type is of type \a void, the \a value member
// constant is set to \a true, the nested type definition \a Type is \a TrueType, and the
// class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsVoid<void>::value          // Evaluates to 'true'
   mtrc::numeric::IsVoid<const void>::Type     // Results in TrueType
   mtrc::numeric::IsVoid<const volatile void>  // Is derived from TrueType
   mtrc::numeric::IsVoid<int>::value           // Evaluates to 'false'
   mtrc::numeric::IsVoid<const char>::Type     // Results in FalseType
   mtrc::numeric::IsVoid<volatile float>       // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsVoid : public BoolConstant<std::is_void<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsVoid type trait.
// \ingroup type_traits
//
// The IsVoid_v variable template provides a convenient shortcut to access the nested \a value
// of the IsVoid class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsVoid<T>::value;
   constexpr bool value2 = mtrc::numeric::IsVoid_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsVoid_v = IsVoid<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
