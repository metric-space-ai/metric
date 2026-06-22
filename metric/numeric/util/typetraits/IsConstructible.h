// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISCONSTRUCTIBLE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISCONSTRUCTIBLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <type_traits>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS ISCONSTRUCTIBLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsConstructible type trait tests whether the expression

   \code
   T obj( std::declval<Args>()... );
   \endcode

// is well formed. If an object of type \a T can be created in this way, the \a value member
// constant is set to \a true, the nested type definition \a Type is set to \a TrueType, and
// the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType and the class derives from \a FalseType.
*/
template <typename T, typename... Args>
struct IsConstructible : public BoolConstant<std::is_constructible<T, Args...>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsConstructible type trait.
// \ingroup type_traits
//
// The IsConstructible_v variable template provides a convenient shortcut to access the nested
// \a value of the IsConstructible class template. For instance, given the types \a T1, \a T2
// and \a T3 the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsConstructible<T1,T2,T3>::value;
   constexpr bool value2 = mtrc::numeric::IsConstructible_v<T1,T2,T3>;
   \endcode
*/
template <typename T, typename... Args> constexpr bool IsConstructible_v = IsConstructible<T>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS ISNOTHROWCONSTRUCTIBLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsNothrowConstructible type trait tests whether the expression

   \code
   T obj( std::declval<Args>()... );
   \endcode

// is well formed and guaranteed to not throw an exception (i.e. noexcept). If an object of type
// \a T can be created in this way, the \a value member constant is set to \a true, the nested
// type definition \a Type is set to \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType and the class derives from \a FalseType.
*/
template <typename T, typename... Args>
struct IsNothrowConstructible : public BoolConstant<std::is_nothrow_constructible<T, Args...>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsNothrowConstructible type trait.
// \ingroup type_traits
//
// The IsNothrowConstructible_v variable template provides a convenient shortcut to access the
// nested \a value of the IsNothrowConstructible class template. For instance, given the types
// \a T1, \a T2 and \a T3 the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsNothrowConstructible<T1,T2,T3>::value;
   constexpr bool value2 = mtrc::numeric::IsNothrowConstructible_v<T1,T2,T3>;
   \endcode
*/
template <typename T, typename... Args> constexpr bool IsNothrowConstructible_v = IsNothrowConstructible<T>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS ISDEFAULTCONSTRUCTIBLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsDefaultConstructible type trait tests whether the expression

   \code
   T obj;
   \endcode

// is well formed. If an object of type \a T can be default constructed, the \a value member
// constant is set to \a true, the nested type definition \a Type is set to \a TrueType, and
// the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType and the class derives from \a FalseType.
*/
template <typename T> struct IsDefaultConstructible : public BoolConstant<std::is_default_constructible<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsDefaultConstructible type trait.
// \ingroup type_traits
//
// The IsDefaultConstructible_v variable template provides a convenient shortcut to access the
// nested \a value of the IsDefaultConstructible class template. For instance, given the type
// \a T the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsDefaultConstructible<T>::value;
   constexpr bool value2 = mtrc::numeric::IsDefaultConstructible_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsDefaultConstructible_v = IsDefaultConstructible<T>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS ISNOTHROWDEFAULTCONSTRUCTIBLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsNothrowDefaultConstructible type trait tests whether the expression

   \code
   T obj;
   \endcode

// is well formed and guaranteed to not throw an exception (i.e. noexcept). If an object of type
// \a T can be default constructed, the \a value member constant is set to \a true, the nested
// type definition \a Type is set to \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType and the class derives from \a FalseType.
*/
template <typename T>
struct IsNothrowDefaultConstructible : public BoolConstant<std::is_nothrow_default_constructible<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsNothrowDefaultConstructible type trait.
// \ingroup type_traits
//
// The IsNothrowDefaultConstructible_v variable template provides a convenient shortcut to access
// the nested \a value of the IsNothrowDefaultConstructible class template. For instance, given
// the type \a T the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsNothrowDefaultConstructible<T>::value;
   constexpr bool value2 = mtrc::numeric::IsNothrowDefaultConstructible_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsNothrowDefaultConstructible_v = IsNothrowDefaultConstructible<T>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS ISCOPYCONSTRUCTIBLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsCopyConstructible type trait tests whether the expression

   \code
   T obj( std::declval<T>() );
   \endcode

// is well formed. If an object of type \a T can be copy constructed, the \a value member
// constant is set to \a true, the nested type definition \a Type is set to \a TrueType, and
// the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType and the class derives from \a FalseType.
*/
template <typename T> struct IsCopyConstructible : public BoolConstant<std::is_copy_constructible<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsCopyConstructible type trait.
// \ingroup type_traits
//
// The IsCopyConstructible_v variable template provides a convenient shortcut to access the
// nested \a value of the IsCopyConstructible class template. For instance, given the type
// \a T the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsCopyConstructible<T>::value;
   constexpr bool value2 = mtrc::numeric::IsCopyConstructible_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsCopyConstructible_v = IsCopyConstructible<T>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS ISNOTHROWCOPYCONSTRUCTIBLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsNothrowCopyConstructible type trait tests whether the expression

   \code
   T obj( std::declval<T>() );
   \endcode

// is well formed and guaranteed to not throw an exception (i.e. noexcept). If an object of type
// \a T can be copy constructed, the \a value member constant is set to \a true, the nested type
// definition \a Type is set to \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType and the class derives from \a FalseType.
*/
template <typename T> struct IsNothrowCopyConstructible : public BoolConstant<std::is_copy_constructible<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsNothrowCopyConstructible type trait.
// \ingroup type_traits
//
// The IsNothrowCopyConstructible_v variable template provides a convenient shortcut to access
// the nested \a value of the IsNothrowCopyConstructible class template. For instance, given
// the type \a T the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsNothrowCopyConstructible<T>::value;
   constexpr bool value2 = mtrc::numeric::IsNothrowCopyConstructible_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsNothrowCopyConstructible_v = IsNothrowCopyConstructible<T>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS ISMOVECONSTRUCTIBLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsMoveConstructible type trait tests whether the expression

   \code
   T obj( std::move( std::declval<T>() ) );
   \endcode

// is well formed. If an object of type \a T can be move constructed, the \a value member
// constant is set to \a true, the nested type definition \a Type is set to \a TrueType, and
// the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType and the class derives from \a FalseType.
*/
template <typename T> struct IsMoveConstructible : public BoolConstant<std::is_move_constructible<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMoveConstructible type trait.
// \ingroup type_traits
//
// The IsMoveConstructible_v variable template provides a convenient shortcut to access the
// nested \a value of the IsMoveConstructible class template. For instance, given the type
// \a T the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMoveConstructible<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMoveConstructible_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMoveConstructible_v = IsMoveConstructible<T>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS ISNOTHROWMOVECONSTRUCTIBLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsNothrowMoveConstructible type trait tests whether the expression

   \code
   T obj( std::move( std::declval<T>() ) );
   \endcode

// is well formed and guaranteed to not throw an exception (i.e. noexcept). If an object of type
// \a T can be move constructed, the \a value member constant is set to \a true, the nested type
// definition \a Type is set to \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType and the class derives from \a FalseType.
*/
template <typename T>
struct IsNothrowMoveConstructible : public BoolConstant<std::is_nothrow_move_constructible<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsNothrowMoveConstructible type trait.
// \ingroup type_traits
//
// The IsNothrowMoveConstructible_v variable template provides a convenient shortcut to access
// the nested \a value of the IsNothrowMoveConstructible class template. For instance, given
// the type \a T the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsNothrowMoveConstructible<T>::value;
   constexpr bool value2 = mtrc::numeric::IsNothrowMoveConstructible_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsNothrowMoveConstructible_v = IsNothrowMoveConstructible<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
