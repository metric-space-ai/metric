// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISASSIGNABLE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISASSIGNABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <type_traits>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS ISASSIGNABLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsAssignable type trait tests whether the expression

   \code
   std::declval<T>() = std::declval<U>();
   \endcode

// is well formed. If an object of type \a U can be assigned to an object of type \a T in this
// way, the \a value member constant is set to \a true, the nested type definition \a Type is set
// to \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType and the class derives from \a FalseType.
*/
template <typename T, typename U> struct IsAssignable : public BoolConstant<std::is_assignable<T, U>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsAssignable type trait.
// \ingroup type_traits
//
// The IsAssignable_v variable template provides a convenient shortcut to access the nested
// \a value of the IsAssignable class template. For instance, given the types \a T and \a U
// the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsAssignable<T,U>::value;
   constexpr bool value2 = mtrc::numeric::IsAssignable_v<T,U>;
   \endcode
*/
template <typename T, typename U> constexpr bool IsAssignable_v = IsAssignable<T, U>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS ISNOTHROWASSIGNABLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsNothrowAssignable type trait tests whether the expression

   \code
   std::declval<T>() = std::declval<U>();
   \endcode

// is well formed and guaranteed to not throw an exception (i.e. noexcept). If an object of type
// \a U can be assigned to an object of type \a T in this way, the \a value member constant is set
// to \a true, the nested type definition \a Type is set to \a TrueType, and the class derives
// from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType and the class
// derives from \a FalseType.
*/
template <typename T, typename U>
struct IsNothrowAssignable : public BoolConstant<std::is_nothrow_assignable<T, U>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsNothrowAssignable type trait.
// \ingroup type_traits
//
// The IsNothrowAssignable_v variable template provides a convenient shortcut to access the
// nested \a value of the IsNothrowAssignable class template. For instance, given the types
// \a T and \a U the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsNothrowAssignable<T,U>::value;
   constexpr bool value2 = mtrc::numeric::IsNothrowAssignable_v<T,U>;
   \endcode
*/
template <typename T, typename U> constexpr bool IsNothrowAssignable_v = IsNothrowAssignable<T, U>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS ISCOPYASSIGNABLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsCopyAssignable type trait tests whether the expression

   \code
   std::declval<T>() = std::declval<T>();
   \endcode

// is well formed. If an object of type \a T can be copy assigned to another object of type \a T,
// the \a value member constant is set to \a true, the nested type definition \a Type is set to
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType and the class derives from \a FalseType.
*/
template <typename T> struct IsCopyAssignable : public BoolConstant<std::is_copy_assignable<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsCopyAssignable type trait.
// \ingroup type_traits
//
// The IsCopyAssignable_v variable template provides a convenient shortcut to access the nested
// \a value of the IsCopyAssignable class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsCopyAssignable<T>::value;
   constexpr bool value2 = mtrc::numeric::IsCopyAssignable_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsCopyAssignable_v = IsCopyAssignable<T>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS ISNOTHROWCOPYASSIGNABLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsNothrowCopyAssignable type trait tests whether the expression

   \code
   std::declval<T>() = std::declval<T>();
   \endcode

// is well formed and guaranteed to not throw an exception (i.e. noexcept). If an object of
// type \a T can be copy assigned to another object of type \a T in this way, the \a value
// member constant is set to \a true, the nested type definition \a Type is set to \a TrueType,
// and the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType and the class derives from \a FalseType.
*/
template <typename T>
struct IsNothrowCopyAssignable : public BoolConstant<std::is_nothrow_copy_assignable<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsNothrowCopyAssignable type trait.
// \ingroup type_traits
//
// The IsNothrowCopyAssignable_v variable template provides a convenient shortcut to access the
// nested \a value of the IsNothrowCopyAssignable class template. For instance, given the type
// \a T the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsNothrowCopyAssignable<T>::value;
   constexpr bool value2 = mtrc::numeric::IsNothrowCopyAssignable_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsNothrowCopyAssignable_v = IsNothrowCopyAssignable<T>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS ISMOVEASSIGNABLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsMoveAssignable type trait tests whether the expression

   \code
   std::declval<T>() = std::move( std::declval<T>() );
   \endcode

// is well formed. If an object of type \a T can be move assigned to another object of type \a T,
// the \a value member constant is set to \a true, the nested type definition \a Type is set to
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType and the class derives from \a FalseType.
*/
template <typename T> struct IsMoveAssignable : public BoolConstant<std::is_move_assignable<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMoveAssignable type trait.
// \ingroup type_traits
//
// The IsMoveAssignable_v variable template provides a convenient shortcut to access the nested
// \a value of the IsMoveAssignable class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMoveAssignable<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMoveAssignable_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMoveAssignable_v = IsMoveAssignable<T>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS ISNOTHROWMOVEASSIGNABLE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsNothrowMoveAssignable type trait tests whether the expression

   \code
   std::declval<T>() = std::move( std::declval<T>() );
   \endcode

// is well formed and guaranteed to not throw an exception (i.e. noexcept). If an object of
// type \a T can be move assigned to another object of type \a T in this way, the \a value
// member constant is set to \a true, the nested type definition \a Type is set to \a TrueType,
// and the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType and the class derives from \a FalseType.
*/
template <typename T>
struct IsNothrowMoveAssignable : public BoolConstant<std::is_nothrow_move_assignable<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsNothrowMoveAssignable type trait.
// \ingroup type_traits
//
// The IsNothrowMoveAssignable_v variable template provides a convenient shortcut to access the
// nested \a value of the IsNothrowMoveAssignable class template. For instance, given the type
// \a T the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsNothrowMoveAssignable<T>::value;
   constexpr bool value2 = mtrc::numeric::IsNothrowMoveAssignable_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsNothrowMoveAssignable_v = IsNothrowMoveAssignable<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
