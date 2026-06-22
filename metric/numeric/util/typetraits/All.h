// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ALL_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ALL_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/mpl/And.h>

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
// all given types \a Ts. If the expression

   \code
   And_t< TypeTrait<Ts>... >::value
   \endcode

// evaluates to \a true, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType. Examples:

   \code
   mtrc::numeric::All< IsIntegral, int, short, long >::value      // Evaluates to 'true'
   mtrc::numeric::All< IsPointer, int*, float* >::Type            // Results in TrueType
   mtrc::numeric::All< IsCharacter, char, signed char, wchar_t >  // Is derived from TrueType
   mtrc::numeric::All< IsIntegral, int, float, double >::value    // Evaluates to 'false'
   mtrc::numeric::All< IsPointer, int*, float& >::Type            // Results in FalseType
   mtrc::numeric::All< IsCharacter, char, signed int, wchar_t >   // Is derived from FalseType
   \endcode
*/
template <template <typename> class TypeTrait // Type trait to be evaluated on all operands
		  ,
		  typename T1 // Type of the first mandatory operand
		  ,
		  typename T2 // Type of the second mandatory operand
		  ,
		  typename... Ts> // Types of the optional operands
struct All : public BoolConstant<And_t<TypeTrait<T1>, TypeTrait<T2>, TypeTrait<Ts>...>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the All type trait.
// \ingroup type_traits
//
// The All_v variable template provides a convenient shortcut to access the nested \a value
// of the All class template. For instance, given the type trait \a TypeTrait and the two
// types \a T1 and \a T2 the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::All<TypeTrait,T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::All_v<TypeTrait,T1,T2>;
   \endcode
*/
template <template <typename> class TypeTrait // Type trait to be evaluated on all operands
		  ,
		  typename T1 // Type of the first mandatory operand
		  ,
		  typename T2 // Type of the second mandatory operand
		  ,
		  typename... Ts> // Types of the optional operands
constexpr bool All_v = All<TypeTrait, T1, T2, Ts...>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
