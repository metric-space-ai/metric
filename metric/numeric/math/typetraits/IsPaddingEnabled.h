// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISPADDINGENABLED_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISPADDINGENABLED_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/AlwaysFalse.h>
#include <metric/numeric/util/typetraits/HasMember.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the IsPaddingEnabled type trait.
// \ingroup math_type_traits
*/
METRIC_NUMERIC_CREATE_HAS_DATA_OR_FUNCTION_MEMBER_TYPE_TRAIT(IsPaddingEnabledHelper1, paddingEnabled);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the IsPaddingEnabled type trait.
// \ingroup math_type_traits
*/
template <typename T> struct IsPaddingEnabledHelper2 {
	static constexpr bool test(bool (*fnc)()) { return fnc(); }
	static constexpr bool test(bool b) { return b; }
	static constexpr bool value = test(T::paddingEnabled);
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for data types.
// \ingroup math_type_traits
//
// This type trait queries the nested \a paddingEnabled member of the given data type \a T, which
// indicates whether the type provides support for padding (i.e. can properly deal with zeros).
// If the type is supporting padding, the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
// Examples:

   \code
   struct A { static constexpr bool paddingEnabled = true; };
   struct B { static constexpr bool paddingEnabled() { return true; } };

   struct C {};
   struct D { static constexpr bool paddingEnabled = false; };
   struct E { static constexpr bool paddingEnabled() { return false; } };

   mtrc::numeric::IsPaddingEnabled< A >::value  // Evaluates to 1
   mtrc::numeric::IsPaddingEnabled< A >::Type   // Results in TrueType
   mtrc::numeric::IsPaddingEnabled< B >         // Is derived from TrueType
   mtrc::numeric::IsPaddingEnabled< C >::value  // Evaluates to 0
   mtrc::numeric::IsPaddingEnabled< D >::Type   // Results in FalseType
   mtrc::numeric::IsPaddingEnabled< E >         // Is derived from FalseType
   \endcode
*/
template <typename T>
struct IsPaddingEnabled
	: public BoolConstant<If_t<IsPaddingEnabledHelper1_v<T>, IsPaddingEnabledHelper2<T>, AlwaysFalse<T>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsPaddingEnabled type trait.
// \ingroup math_type_traits
//
// The IsPaddingEnabled_v variable template provides a convenient shortcut to access the nested
// \a value of the IsPaddingEnabled class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsPaddingEnabled<T>::value;
   constexpr bool value2 = mtrc::numeric::IsPaddingEnabled_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsPaddingEnabled_v = IsPaddingEnabled<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
