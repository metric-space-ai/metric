// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ANY_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ANY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/mpl/Or.h>

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
// This type trait determines whether the given type trait \a TypeTrait evaluates to \a true for
// at least one of the given types \a Ts. If the expression

   \code
   Or_t< TypeTrait<Ts>... >::value
   \endcode

// evaluates to \a true, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType. Examples:

   \code
   mtrc::numeric::Any< IsIntegral, int, float >::value     // Evaluates to 'true'
   mtrc::numeric::Any< IsPointer, int&, float* >::Type     // Results in TrueType
   mtrc::numeric::Any< IsCharacter, float, wchar_t >       // Is derived from TrueType
   mtrc::numeric::Any< IsIntegral, float, double >::value  // Evaluates to 'false'
   mtrc::numeric::Any< IsPointer, int, float& >::Type      // Results in FalseType
   mtrc::numeric::Any< IsCharacter, int, double >          // Is derived from FalseType
   \endcode
*/
template <template <typename> class TypeTrait // Type trait to be evaluated on all operands
		  ,
		  typename T1 // Type of the first mandatory operand
		  ,
		  typename T2 // Type of the second mandatory operand
		  ,
		  typename... Ts> // Types of the optional operands
struct Any : public BoolConstant<Or_t<TypeTrait<T1>, TypeTrait<T2>, TypeTrait<Ts>...>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Any type trait.
// \ingroup type_traits
//
// The Any_v variable template provides a convenient shortcut to access the nested \a value
// of the Any class template. For instance, given the type trait \a TypeTrait and the two
// types \a T1 and \a T2 the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::Any<TypeTrait,T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::Any_v<TypeTrait,T1,T2>;
   \endcode
*/
template <template <typename> class TypeTrait // Type trait to be evaluated on all operands
		  ,
		  typename T1 // Type of the first mandatory operand
		  ,
		  typename T2 // Type of the second mandatory operand
		  ,
		  typename... Ts> // Types of the optional operands
constexpr bool Any_v = Any<TypeTrait, T1, T2, Ts...>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
