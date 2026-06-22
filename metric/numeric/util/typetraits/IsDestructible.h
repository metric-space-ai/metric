// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISDESTRUCTIBLE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISDESTRUCTIBLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <type_traits>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS ISDESTRUCTIBLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsDestructible type trait tests whether the expression

   \code
   std::declval<U&>().~U();
   \endcode

// is well formed, where \a U represents the type \a T stripped of all extents. If an object of
// type \a T can be destroyed in this way, the \a value member constant is set to \a true, the
// nested type definition \a Type is set to \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType and the class derives from
// \a FalseType.
*/
template <typename T> struct IsDestructible : public BoolConstant<std::is_destructible<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsDestructible type trait.
// \ingroup type_traits
//
// The IsDestructible_v variable template provides a convenient shortcut to access the nested
// \a value of the IsDestructible class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsDestructible<T>::value;
   constexpr bool value2 = mtrc::numeric::IsDestructible_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsDestructible_v = IsDestructible<T>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS ISNOTHROWDESTRUCTIBLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsDestructible type trait tests whether the expression

   \code
   std::declval<U&>().~U();
   \endcode

// is well formed and guaranteed to not throw an exception (i.e. noexcept), where \a U represents
// the type \a T stripped of all extents. If an object of type \a T can be destroyed in this way,
// the \a value member constant is set to \a true, the nested type definition \a Type is set to
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType and the class derives from \a FalseType.
*/
template <typename T> struct IsNothrowDestructible : public BoolConstant<std::is_nothrow_destructible<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsNothrowDestructible type trait.
// \ingroup type_traits
//
// The IsNothrowDestructible_v variable template provides a convenient shortcut to access the
// nested \a value of the IsNothrowDestructible class template. For instance, given the type
// \a T the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsNothrowDestructible<T>::value;
   constexpr bool value2 = mtrc::numeric::IsNothrowDestructible_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsNothrowDestructible_v = IsNothrowDestructible<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
