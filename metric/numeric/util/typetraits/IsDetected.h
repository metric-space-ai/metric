// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISDETECTED_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISDETECTED_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/Void.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Default result type for the Detected_t type trait.
// \ingroup type_traits
*/
struct NoneSuch {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the IsDetected type trait.
// \ingroup type_traits
*/
template <typename Default, typename AlwaysVoid, template <typename...> class OP, typename... Ts> struct Detector {
	using Value_t = FalseType;
	using Type = Default;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Detector class template for a successful evaluation of \a OP.
// \ingroup type_traits
*/
template <typename Default, template <typename...> class OP, typename... Ts>
struct Detector<Default, Void_t<OP<Ts...>>, OP, Ts...> {
	using Value_t = TrueType;
	using Type = OP<Ts...>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time detection of type properties.
// \ingroup type_traits
//
// The DetectedOr type trait determines if the given \a OP can be evaluated for the given types
// \a Ts. In case \a OP can be successfully evaluated, Detected_t evaluates to OP<Ts...>. Otherwise
// it evaluates to Default.

   \code
   template<class T>
   using Diff_t = typename T::difference_type;

   struct C { using difference_type = int; };
   struct D { };

   template< typename T >
   using difference_type = typename DetectedOr<ptrdiff_t, Diff_t, T>::Type;

   static_assert( IsSame_v< difference_type<C>, int >      , "C's difference_type should be int!" );
   static_assert( IsSame_v< difference_type<D>, ptrdiff_t >, "D's difference_type should be ptrdiff_t!");
   \endcode
*/
template <typename Default, template <typename...> class OP, typename... Ts>
using DetectedOr = Detector<Default, void, OP, Ts...>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time detection of type properties.
// \ingroup type_traits
//
// The IsDetected type trait determines if the given \a OP can be evaluated for the given types
// \a Ts. In case \a OP can be successfully evaluated, IsDetected is an alias for TrueType.
// Otherwise it is an alias for FalseType.

   \code
   template<class T>
   using CopyAssign_t = decltype( std::declval<T&>() = std::declval<const T&>() );

   struct A { };
   struct B { void operator=( const B& ) = delete; };

   static_assert( IsDetected_v<CopyAssign_t, A> , "A should be copy assignable!" );
   static_assert( !IsDetected_v<CopyAssign_t, B>, "B should not be copy assignable!");
   \endcode

   \code
   template<class T>
   using Diff_t = typename T::difference_type;

   struct C { using difference_type = int; };
   struct D { };

   static_assert( IsDetected_v<Diff_t, C> , "C should have a difference type!" );
   static_assert( !IsDetected_v<Diff_t, D>, "D should not have a difference type!");
   \endcode
*/
template <template <typename...> class OP, typename... Ts>
using IsDetected = typename DetectedOr<NoneSuch, OP, Ts...>::Value_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time detection of type properties.
// \ingroup type_traits
//
// The Detected_t type trait determines if the given \a OP can be evaluated for the given types
// \a Ts. In case \a OP can be successfully evaluated, DetectedOr evaluates to OP<Ts...>. Otherwise
// it evaluates to NoneSuch.

   \code
   template<class T>
   using Diff_t = typename T::difference_type;

   struct C { using difference_type = int; };
   struct D { };

   static_assert( IsSame_v< Detected_t<Diff_t, C>, int >     , "C's difference_type should be int!" );
   static_assert( IsSame_v< Detected_t<Diff_t, D>, NoneSuch >, "D should not have a difference type!");
   \endcode
*/
template <template <typename...> class OP, typename... Ts>
using Detected_t = typename DetectedOr<NoneSuch, OP, Ts...>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsDetected type trait.
// \ingroup type_traits
//
// The IsDetected_v variable template provides a convenient shortcut to access the nested
// \a value of the IsDetected class template. For instance, given the template \a OP and the
// two types \a T1 and \a T2 the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsDetected<OP,T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::IsDetected_v<OP,T1,T2>;
   \endcode
*/
template <template <typename...> class OP, typename... Ts> constexpr bool IsDetected_v = IsDetected<OP, Ts...>::value;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the DetectedOr type trait.
// \ingroup type_traits
//
// The DetectedOr_t variable template provides a convenient shortcut to access the nested
// \a Type of the DetectedOr class template. For instance, given the type \a Default, the
// template \a OP and the two types \a T1 and \a T2 the following two statements are identical:

   \code
   using Type1 = typename mtrc::numeric::DetectedOr<Default,OP,T1,T2>::Type;
   using Type2 = mtrc::numeric::DetectedOr_t<Default,OP,T1,T2>;
   \endcode
*/
template <typename Default, template <typename...> class OP, typename... Ts>
using DetectedOr_t = typename DetectedOr<Default, OP, Ts...>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
