// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSIMDENABLED_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSIMDENABLED_H
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
/*!\brief First auxiliary helper struct for the IsSIMDEnabled type trait.
// \ingroup math_type_traits
*/
METRIC_NUMERIC_CREATE_HAS_DATA_OR_FUNCTION_MEMBER_TYPE_TRAIT(HasSIMDEnabled, simdEnabled);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the IsSIMDEnabled type trait.
// \ingroup math_type_traits
*/
template <typename T, typename... Args> struct GetSIMDEnabled {
	static constexpr bool test(bool (*fnc)()) { return fnc(); }
	static constexpr bool test(bool b) { return b; }
	static constexpr bool value = test(T::template simdEnabled<Args...>);
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the GetSIMDEnabled class template for non-templates.
// \ingroup math_type_traits
*/
template <typename T> struct GetSIMDEnabled<T> {
	static constexpr bool test(bool (*fnc)()) { return fnc(); }
	static constexpr bool test(bool b) { return b; }
	static constexpr bool value = test(T::simdEnabled);
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for data types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given data type \a T is a SIMD-enabled data type
// (i.e. provides the according SIMD-related member functions, such as \c load(), store(), etc).
// If the type is SIMD-enabled, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T, typename... Args>
struct IsSIMDEnabled
	: public BoolConstant<If_t<HasSIMDEnabled_v<T>, GetSIMDEnabled<T, Args...>, AlwaysFalse<T>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSIMDEnabled type trait.
// \ingroup math_type_traits
//
// The IsSIMDEnabled_v variable template provides a convenient shortcut to access the nested
// \a value of the IsSIMDEnabled class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSIMDEnabled<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSIMDEnabled_v<T>;
   \endcode
*/
template <typename T, typename... Args> constexpr bool IsSIMDEnabled_v = IsSIMDEnabled<T, Args...>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
